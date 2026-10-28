#define BOOST_PROCESS_USE_STD_FS

#include <fb/bot/cluster/agent.h>
#include <fb/bot/cluster/link.h>
#include <fb/console.h>

#include <boost/process.hpp>

#include <algorithm>
#include <chrono>
#include <deque>
#include <filesystem>
#include <format>
#include <fstream>
#include <mutex>
#include <numeric>
#include <thread>

using namespace fb::bot::cluster;
using namespace std::chrono_literals;

struct agent::child
{
    std::string              run_id;
    boost::process::ipstream output;
    boost::process::opstream input;
    boost::process::child    process;
    std::thread              reader;
    std::mutex               mutex;
    std::deque<std::string>  tail;
    std::ofstream            log;
    bool                     stopping = false;
};

agent::agent(boost::asio::io_context& context, options options) :
    _context(context),
    _options(std::move(options))
{ }

agent::~agent()
{
    if (this->_child == nullptr)
        return;

    if (this->_child->process.running())
        this->_child->process.terminate();
    if (this->_child->reader.joinable())
        this->_child->reader.join();
}

boost::asio::awaitable<void> agent::run()
{
    auto delay = std::chrono::seconds{1};
    auto timer = boost::asio::steady_timer{this->_context};
    while (this->_running)
    {
        auto connected = co_await this->session();
        this->stop();
        if (this->_running == false)
            break;

        delay = connected ? std::chrono::seconds{1} : std::min(delay * 2, std::chrono::seconds{30});
        fb::console::puts("agent: master disconnected, retry in {}s", delay.count());
        timer.expires_after(delay);
        co_await timer.async_wait(boost::asio::use_awaitable);
    }
    this->_context.stop();
}

boost::asio::awaitable<bool> agent::session()
{
    auto address = std::string_view{this->_options.master};
    if (address.starts_with("ws://"))
        address.remove_prefix(5);

    auto slash = address.find('/');
    if (slash != std::string_view::npos)
        address = address.substr(0, slash);

    auto colon = address.rfind(':');
    auto host  = std::string{address.substr(0, colon)};
    auto port  = colon == std::string_view::npos ? std::string{"8089"} : std::string{address.substr(colon + 1)};
    auto error = boost::system::error_code{};

    auto resolver = boost::asio::ip::tcp::resolver{this->_context};
    auto results =
        co_await resolver.async_resolve(host, port, boost::asio::redirect_error(boost::asio::use_awaitable, error));
    if (error)
        co_return false;

    auto stream = channel::stream{this->_context};
    boost::beast::get_lowest_layer(stream).expires_after(10s);
    co_await boost::beast::get_lowest_layer(stream).async_connect(
        results,
        boost::asio::redirect_error(boost::asio::use_awaitable, error));
    if (error)
        co_return false;

    boost::beast::get_lowest_layer(stream).expires_never();
    stream.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));
    co_await stream.async_handshake(std::format("{}:{}", host, port),
                                    "/ws/agent",
                                    boost::asio::redirect_error(boost::asio::use_awaitable, error));
    if (error)
        co_return false;

    this->_channel = std::make_shared<channel>(std::move(stream));

    auto pid          = static_cast<uint32_t>(boost::this_process::get_id());
    auto hello        = Json::Value{};
    hello["type"]     = "hello";
    hello["token"]    = this->_options.token;
    hello["name"]     = this->_options.name;
    hello["host"]     = boost::asio::ip::host_name();
    hello["pid"]      = pid;
    hello["seats"]    = this->_options.seats;
    hello["max_bots"] = this->_options.max_bots;
    hello["version"]  = std::format("{} {}", __DATE__, __TIME__);
    this->_channel->send(hello);
    fb::console::puts("agent: connected to {}", this->_options.master);

    boost::asio::co_spawn(
        this->_context,
        [target = std::weak_ptr<channel>(this->_channel)]() -> boost::asio::awaitable<void> {
            auto timer = boost::asio::steady_timer{co_await boost::asio::this_coro::executor};
            while (true)
            {
                timer.expires_after(2s);
                co_await timer.async_wait(boost::asio::use_awaitable);

                auto link = target.lock();
                if (link == nullptr || link->closed())
                    break;

                auto beat    = Json::Value{};
                beat["type"] = "heartbeat";
                link->send(beat);
            }
        },
        boost::asio::detached);

    while (true)
    {
        auto message = co_await this->_channel->read();
        if (message.has_value() == false)
            break;

        this->handle(message.value());
    }

    this->_channel.reset();
    co_return true;
}

void agent::handle(const Json::Value& message)
{
    auto type = message["type"].asString();
    if (type == "error")
    {
        fb::console::puts("agent: refused by master ({})", message["message"].asString());
    }
    else if (type == "welcome")
    {
        this->_id = message["agent_id"].asUInt();
        fb::console::puts("agent: registered as #{}", this->_id);
    }
    else if (type == "run_suites")
    {
        auto suites = std::string{};
        for (auto& suite : message["suites"])
        {
            suites += (suites.empty() ? "" : ",") + suite.asString();
        }

        auto arguments = std::vector<std::string>{"--mode",
                                                  "integration",
                                                  "--config",
                                                  this->_options.config,
                                                  "--cluster",
                                                  "--test",
                                                  suites,
                                                  "--set",
                                                  std::format("cluster:seat_base={}", message["seat_base"].asUInt()),
                                                  "--set",
                                                  std::format("thread:logic={}", this->_options.seats)};
        if (message["client"].asString().empty() == false)
        {
            arguments.push_back("--client");
            arguments.push_back(message["client"].asString());
        }
        if (message["target"].asString().empty() == false)
        {
            arguments.push_back("--target");
            arguments.push_back(message["target"].asString());
        }
        this->spawn(message["run_id"].asString(), arguments);
    }
    else if (type == "run_load")
    {
        auto arguments = std::vector<std::string>{
            "--mode",
            "load",
            "--config",
            this->_options.config,
            "--cluster",
            "--set",
            std::format("spawn_count={}", message["users"].asUInt()),
            "--set",
            std::format("spawn_per_interval={}", std::max(1u, message["rate"].asUInt())),
            "--set",
            "interval=1000",
            "--set",
            std::format("load:process={}", message["process"].asUInt()),
            "--set",
            std::format("load:account_offset={}", message["offset"].asUInt()),
            "--set",
            "load:metrics_interval_ms=2000",
        };
        if (message["client"].asString().empty() == false)
        {
            arguments.push_back("--client");
            arguments.push_back(message["client"].asString());
        }
        if (message["target"].asString().empty() == false)
        {
            arguments.push_back("--target");
            arguments.push_back(message["target"].asString());
        }
        this->spawn(message["run_id"].asString(), arguments);
    }
    else if (type == "scale" || type == "sync")
    {
        this->write(message);
    }
    else if (type == "stop")
    {
        this->stop();
    }
    else if (type == "quit")
    {
        this->_running = false;
        this->stop();
        if (this->_channel != nullptr)
            this->_channel->close();
    }
}

void agent::spawn(const std::string& run_id, const std::vector<std::string>& arguments)
{
    if (this->_child != nullptr)
    {
        auto refused      = Json::Value{};
        refused["type"]   = "child_exit";
        refused["run_id"] = run_id;
        refused["code"]   = -1;
        refused["log"]    = "agent is already running a child process";
        if (this->_channel != nullptr)
            this->_channel->send(refused);
        return;
    }

    auto created    = std::make_shared<child>();
    created->run_id = run_id;
    std::filesystem::create_directories("logs/cluster");
    created->log.open(std::format("logs/cluster/{}-{}.log", run_id, this->_options.name), std::ios::app);
    try
    {
        created->process = boost::process::child(this->_options.executable,
                                                 boost::process::args(arguments),
                                                 (boost::process::std_out & boost::process::std_err) > created->output,
                                                 boost::process::std_in < created->input);
    }
    catch (const std::exception& e)
    {
        auto failed      = Json::Value{};
        failed["type"]   = "child_exit";
        failed["run_id"] = run_id;
        failed["code"]   = -1;
        failed["log"]    = e.what();
        if (this->_channel != nullptr)
            this->_channel->send(failed);
        return;
    }

    fb::console::puts(
        "agent: run {} started ({})",
        run_id,
        std::accumulate(arguments.begin(), arguments.end(), std::string{}, [](auto text, const auto& argument) {
            return text + " " + argument;
        }));
    this->_child    = created;
    created->reader = std::thread([this, created]() {
        auto line = std::string{};
        while (std::getline(created->output, line))
        {
            if (line.ends_with('\r'))
                line.pop_back();

            auto message = Json::Value{};
            if (link::parse(line, message))
            {
                boost::asio::post(this->_context, [this, created, message = std::move(message)]() mutable {
                    this->on_output(created, std::move(message));
                });
            }
            else
            {
                auto lock = std::lock_guard(created->mutex);
                created->log << line << '\n';
                created->tail.push_back(line);
                if (created->tail.size() > 200)
                    created->tail.pop_front();
            }
        }

        boost::asio::post(this->_context, [this, created]() {
            this->on_exit(created);
        });
    });
}

void agent::write(const Json::Value& message)
{
    if (this->_child == nullptr || this->_child->stopping)
        return;

    this->_child->input << link::MARKER << link::serialize(message) << std::endl;
}

void agent::stop()
{
    if (this->_child == nullptr || this->_child->stopping)
        return;

    auto stopped    = Json::Value{};
    stopped["type"] = "stop";
    this->write(stopped);
    this->_child->stopping = true;

    auto timer = std::make_shared<boost::asio::steady_timer>(this->_context, 15s);
    timer->async_wait([timer, target = std::weak_ptr<child>(this->_child)](const boost::system::error_code&) {
        auto found = target.lock();
        if (found != nullptr && found->process.running())
            found->process.terminate();
    });
}

void agent::on_output(const std::shared_ptr<child>& source, Json::Value message)
{
    if (source != this->_child || this->_channel == nullptr)
        return;

    if (message["type"].asString() == "suite_finished" && message["success"].asBool() == false)
    {
        auto lock = std::lock_guard(source->mutex);
        auto log  = std::string{};
        for (auto& line : source->tail)
        {
            log += line + "\n";
        }
        message["log"] = log;
    }

    message["run_id"] = source->run_id;
    this->_channel->send(message);
}

void agent::on_exit(const std::shared_ptr<child>& source)
{
    if (source->reader.joinable())
        source->reader.join();

    auto error = std::error_code{};
    source->process.wait(error);
    auto code = source->process.exit_code();
    fb::console::puts("agent: run {} exited ({})", source->run_id, code);

    if (source == this->_child)
        this->_child.reset();

    if (this->_channel == nullptr)
        return;

    auto exited      = Json::Value{};
    exited["type"]   = "child_exit";
    exited["run_id"] = source->run_id;
    exited["code"]   = code;
    if (code != 0)
    {
        auto lock = std::lock_guard(source->mutex);
        auto log  = std::string{};
        for (auto& line : source->tail)
        {
            log += line + "\n";
        }
        exited["log"] = log;
    }
    this->_channel->send(exited);
}
