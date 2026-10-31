#include <fb/bot/cluster/link.h>
#include <fb/bot/cluster/master.h>
#include <fb/bot/integration/lua_integration_test.h>
#include <fb/config.h>
#include <fb/console.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>

using namespace fb::bot::cluster;
using namespace std::chrono_literals;

namespace http      = boost::beast::http;
namespace websocket = boost::beast::websocket;

master::master(boost::asio::io_context& context, options options) :
    _context(context),
    _options(std::move(options))
{
    this->load_leases();
    this->reload();
}

boost::asio::awaitable<void> master::run()
{
    auto colon    = this->_options.listen.rfind(':');
    auto address  = boost::asio::ip::make_address(this->_options.listen.substr(0, colon));
    auto port     = static_cast<uint16_t>(std::stoul(this->_options.listen.substr(colon + 1)));
    auto acceptor = boost::asio::ip::tcp::acceptor{
        this->_context,
        {address, port}
    };
    fb::console::puts("master: dashboard http://{}", this->_options.listen);

    boost::asio::co_spawn(this->_context, this->tick(), boost::asio::detached);
    while (true)
    {
        auto socket = co_await acceptor.async_accept(boost::asio::use_awaitable);
        boost::asio::co_spawn(this->_context, this->serve(std::move(socket)), boost::asio::detached);
    }
}

boost::asio::awaitable<void> master::serve(boost::asio::ip::tcp::socket socket)
{
    auto error  = boost::system::error_code{};
    auto remote = socket.remote_endpoint(error);
    auto local  = error ? false : remote.address().is_loopback();
    auto stream = boost::beast::tcp_stream{std::move(socket)};
    auto buffer = boost::beast::flat_buffer{};
    while (true)
    {
        auto received = request{};
        stream.expires_after(60s);
        co_await http::async_read(stream,
                                  buffer,
                                  received,
                                  boost::asio::redirect_error(boost::asio::use_awaitable, error));
        if (error)
            co_return;

        if (websocket::is_upgrade(received))
        {
            stream.expires_never();
            auto upgraded = channel::stream{std::move(stream)};
            upgraded.set_option(websocket::stream_base::timeout::suggested(boost::beast::role_type::server));
            co_await upgraded.async_accept(received, boost::asio::redirect_error(boost::asio::use_awaitable, error));
            if (error)
                co_return;

            auto link = std::make_shared<channel>(std::move(upgraded));
            if (received.target() == "/ws/agent")
                co_await this->serve_agent(link, local);
            else
                co_await this->serve_dashboard(link);
            co_return;
        }

        auto sent = this->route(received);
        sent.keep_alive(received.keep_alive());
        sent.prepare_payload();
        co_await http::async_write(stream, sent, boost::asio::redirect_error(boost::asio::use_awaitable, error));
        if (error || sent.keep_alive() == false)
            co_return;
    }
}

boost::asio::awaitable<void> master::serve_agent(std::shared_ptr<channel> link, bool local)
{
    auto hello = co_await link->read();
    if (hello.has_value() == false || (*hello)["type"].asString() != "hello")
    {
        link->close();
        co_return;
    }

    auto refuse = [&link](const std::string& reason) {
        auto refused       = Json::Value{};
        refused["type"]    = "error";
        refused["message"] = reason;
        link->send(refused);
        link->close();
        fb::console::puts("master: agent refused ({})", reason);
    };

    auto token = (*hello)["token"].asString();
    if (this->_options.token.empty() ? local == false : token != this->_options.token)
    {
        refuse("invalid token");
        co_return;
    }

    auto name  = (*hello)["name"].asString();
    auto host  = (*hello)["host"].asString();
    auto key   = std::format("{}@{}", name, host);
    auto found = std::find_if(this->_agents.begin(), this->_agents.end(), [&key](const auto& pair) {
        return pair.second.key == key;
    });
    if (found != this->_agents.end() && found->second.link != nullptr)
    {
        refuse(std::format("duplicate agent {}", key));
        co_return;
    }

    auto id = found != this->_agents.end() ? found->first : this->_next_agent++;
    {
        auto& agent    = this->_agents[id];
        agent.id       = id;
        agent.key      = key;
        agent.name     = name;
        agent.host     = host;
        agent.version  = (*hello)["version"].asString();
        agent.pid      = (*hello)["pid"].asUInt();
        agent.seats    = std::max(1u, (*hello)["seats"].asUInt());
        agent.max_bots = (*hello)["max_bots"].asUInt();
        agent.link     = link;
        agent.busy     = false;
        agent.excluded = false;
        agent.users    = 0;
        agent.bots     = 0;
        agent.batch.clear();
        agent.seen = clock::now();

        auto& entry = this->_leases[key];
        if (entry.process == 0)
        {
            for (auto& [_, other] : this->_leases)
            {
                entry.process = std::max(entry.process, other.process);
            }
            entry.process++;
        }

        if (entry.seats < agent.seats)
        {
            auto base = uint32_t{0};
            for (auto& [_, other] : this->_leases)
            {
                if (other.seats > 0)
                    base = std::max(base, other.seat_base + other.seats + 1);
            }
            entry.seat_base = base;
            entry.seats     = agent.seats;
        }
        this->save_leases();

        auto welcome         = Json::Value{};
        welcome["type"]      = "welcome";
        welcome["agent_id"]  = id;
        welcome["process"]   = entry.process;
        welcome["seat_base"] = entry.seat_base;
        link->send(welcome);
        fb::console::puts("master: agent #{} {} connected (seats={} max_bots={})",
                          id,
                          key,
                          agent.seats,
                          agent.max_bots);
    }

    if (this->_run.has_value() && this->_run->kind == "load")
        this->rebalance();
    else
        this->dispatch();
    this->broadcast();

    while (true)
    {
        auto message = co_await link->read();
        if (message.has_value() == false)
            break;

        auto agent = this->_agents.find(id);
        if (agent == this->_agents.end() || agent->second.link != link)
            break;

        agent->second.seen = clock::now();
        this->handle(agent->second, message.value());
    }

    auto agent = this->_agents.find(id);
    if (agent != this->_agents.end() && agent->second.link == link)
        this->drop(agent->second);
}

boost::asio::awaitable<void> master::serve_dashboard(std::shared_ptr<channel> link)
{
    this->_dashboards.push_back(link);
    link->send(this->snapshot());
    while (true)
    {
        auto message = co_await link->read();
        if (message.has_value() == false)
            break;
    }
    std::erase(this->_dashboards, link);
}

boost::asio::awaitable<void> master::tick()
{
    auto timer = boost::asio::steady_timer{this->_context};
    while (true)
    {
        timer.expires_after(2s);
        co_await timer.async_wait(boost::asio::use_awaitable);

        auto now = clock::now();
        std::erase_if(this->_agents, [now](const auto& pair) {
            return pair.second.link == nullptr && now - pair.second.lost > 10min;
        });

        if (this->_run.has_value() && this->_run->state != "done")
        {
            auto& run   = this->_run.value();
            run.elapsed = std::chrono::duration<double>(now - run.started).count();
            auto merged = stats::entry{};
            auto point  = Json::Value{};
            auto bots   = uint32_t{0};
            auto agents = uint32_t{0};
            auto kills  = uint64_t{0};
            auto deaths = uint64_t{0};
            auto exp    = uint64_t{0};
            for (auto& [name, entry] : run.window)
            {
                if (name.starts_with("goal:") == false && name.starts_with("action:") == false)
                    merged.merge(entry);
            }

            for (auto& [_, agent] : this->_agents)
            {
                if (agent.link != nullptr)
                    agents++;
                if (agent.busy)
                    bots += agent.bots;
                if (agent.metrics["run_id"].asString() == run.id)
                {
                    kills  += agent.metrics["kills"].asUInt64();
                    deaths += agent.metrics["deaths"].asUInt64();
                    exp    += agent.metrics["exp"].asUInt64();
                }
            }

            point["t"]      = std::round(run.elapsed);
            point["rps"]    = merged.count / 2.0;
            point["fps"]    = merged.failures / 2.0;
            point["p50"]    = std::round(merged.percentile(0.50));
            point["p95"]    = std::round(merged.percentile(0.95));
            point["p99"]    = std::round(merged.percentile(0.99));
            point["bots"]   = bots;
            point["agents"] = agents;
            point["kills"]  = static_cast<Json::UInt64>(kills);
            point["deaths"] = static_cast<Json::UInt64>(deaths);
            point["exp"]    = static_cast<Json::UInt64>(exp);
            run.timeline.append(point);
            run.window.clear();

            if (run.kind == "load" && run.state == "running" && run.duration > 0 && run.elapsed >= run.duration * 60.0)
                this->stop();
        }
        this->broadcast();
    }
}

master::response master::route(const request& received)
{
    auto target = std::string{received.target()};
    auto path   = target.substr(0, target.find('?'));
    auto post   = received.method() == http::verb::post;
    auto reply  = [&received](http::status status, const Json::Value& body) {
        auto sent = response{status, received.version()};
        sent.set(http::field::content_type, "application/json; charset=utf-8");
        sent.set(http::field::cache_control, "no-store");
        sent.body() = link::serialize(body);
        return sent;
    };
    auto failed = [&reply](http::status status, const std::string& reason) {
        auto body     = Json::Value{};
        body["error"] = reason;
        return reply(status, body);
    };
    auto body = Json::Value{};
    if (post && received.body().empty() == false && link::decode(received.body(), body) == false)
        return failed(http::status::bad_request, "invalid json");

    if (path.starts_with("/api/") == false)
        return this->file(received, path == "/" ? std::string{"index.html"} : path.substr(1));

    if (path == "/api/state")
        return reply(http::status::ok, this->snapshot());

    if (path == "/api/suites")
        return reply(http::status::ok, this->suites());

    if (path == "/api/targets")
        return reply(http::status::ok, this->targets());

    if (path == "/api/reload" && post)
    {
        auto result = this->reload();
        return reply(result.isMember("error") ? http::status::conflict : http::status::ok, result);
    }

    if (path == "/api/runs" && post)
    {
        auto result = this->start(body);
        return reply(result.isMember("error") ? http::status::conflict : http::status::ok, result);
    }

    if (path == "/api/runs/current/stop" && post)
    {
        this->stop();
        return reply(http::status::ok, Json::Value{Json::objectValue});
    }

    if (path == "/api/runs/current/scale" && post)
    {
        this->scale(body["users"].asUInt());
        return reply(http::status::ok, Json::Value{Json::objectValue});
    }

    if (path == "/api/runs")
        return reply(http::status::ok, this->history());

    if (path.starts_with("/api/agents/") && path.ends_with("/quit") && post)
    {
        auto id =
            static_cast<uint32_t>(std::strtoul(path.c_str() + std::string_view{"/api/agents/"}.size(), nullptr, 10));
        auto agent = this->_agents.find(id);
        if (agent == this->_agents.end() || agent->second.link == nullptr)
            return failed(http::status::not_found, "agent not found");

        auto quit    = Json::Value{};
        quit["type"] = "quit";
        agent->second.link->send(quit);
        return reply(http::status::ok, Json::Value{Json::objectValue});
    }

    if (path.starts_with("/api/runs/"))
    {
        auto rest  = path.substr(std::string_view{"/api/runs/"}.size());
        auto slash = rest.find('/');
        auto id    = rest.substr(0, slash);
        if (id.empty() || id.find("..") != std::string::npos || id.find('\\') != std::string::npos)
            return failed(http::status::bad_request, "invalid run id");

        auto directory = std::filesystem::path{"logs/runs"} / id;
        if (std::filesystem::exists(directory) == false)
            return failed(http::status::not_found, "run not found");

        if (slash != std::string::npos)
        {
            auto name = rest.substr(slash + 1);
            if (name != "summary.json" && name != "timeline.jsonl" && name != "stats.csv")
                return failed(http::status::not_found, "file not found");

            auto input = std::ifstream(directory / name, std::ios::binary);
            auto sent  = response{http::status::ok, received.version()};
            sent.set(http::field::content_type,
                     name.ends_with(".csv") ? "text/csv; charset=utf-8" : "application/json; charset=utf-8");
            sent.set(http::field::content_disposition, std::format("attachment; filename=\"{}-{}\"", id, name));
            sent.body() = std::string{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
            return sent;
        }

        auto summary = Json::Value{};
        auto input   = std::ifstream(directory / "summary.json", std::ios::binary);
        auto text    = std::string{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        link::decode(text, summary);

        auto lines = std::ifstream(directory / "timeline.jsonl", std::ios::binary);
        auto line  = std::string{};
        while (std::getline(lines, line))
        {
            auto point = Json::Value{};
            if (link::decode(line, point))
                summary["timeline"].append(point);
        }
        return reply(http::status::ok, summary);
    }

    return failed(http::status::not_found, "not found");
}

master::response master::file(const request& received, const std::string& path) const
{
    auto sent = response{http::status::ok, received.version()};
    auto root = std::filesystem::weakly_canonical(this->_options.web);
    auto full = std::filesystem::weakly_canonical(root / path);
    auto rel  = full.lexically_relative(root);
    if (rel.empty() || rel.native().starts_with(std::filesystem::path{".."}.native()) ||
        std::filesystem::is_regular_file(full) == false)
    {
        sent.result(http::status::not_found);
        sent.set(http::field::content_type, "text/plain; charset=utf-8");
        sent.body() = "not found";
        return sent;
    }

    auto types = std::map<std::string, std::string>{
        {".html", "text/html; charset=utf-8"       },
        {".js",   "text/javascript; charset=utf-8" },
        {".css",  "text/css; charset=utf-8"        },
        {".json", "application/json; charset=utf-8"},
        {".svg",  "image/svg+xml"                  },
        {".png",  "image/png"                      },
        {".ico",  "image/x-icon"                   },
    };
    auto type  = types.find(full.extension().string());
    auto input = std::ifstream(full, std::ios::binary);
    sent.set(http::field::content_type, type != types.end() ? type->second : "application/octet-stream");
    sent.set(http::field::cache_control, "no-store");
    sent.body() = std::string{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    return sent;
}

Json::Value master::snapshot() const
{
    auto now          = clock::now();
    auto json         = Json::Value{};
    json["type"]      = "state";
    json["revision"]  = this->_revision;
    json["loaded_at"] = this->_loaded_at;
    json["agents"]    = Json::Value{Json::arrayValue};
    for (auto& [id, agent] : this->_agents)
    {
        auto  lease       = this->_leases.find(agent.key);
        auto  seen        = std::chrono::duration<double>(now - agent.seen).count();
        auto& item        = json["agents"].append(Json::Value{});
        item["id"]        = id;
        item["name"]      = agent.name;
        item["host"]      = agent.host;
        item["pid"]       = agent.pid;
        item["seats"]     = agent.seats;
        item["max"]       = agent.max_bots;
        item["bots"]      = agent.bots;
        item["users"]     = agent.users;
        item["seen"]      = std::round(seen * 10.0) / 10.0;
        item["state"]     = agent.link == nullptr ? "lost" : seen > 6.0 ? "slow" : agent.busy ? "run" : "idle";
        item["version"]   = agent.version;
        item["process"]   = lease != this->_leases.end() ? lease->second.process : 0;
        item["seat_base"] = lease != this->_leases.end() ? lease->second.seat_base : 0;
        item["excluded"]  = agent.excluded;
        item["batch"]     = Json::Value{Json::arrayValue};
        for (auto& name : agent.batch)
        {
            item["batch"].append(name);
        }
    }

    json["run"] = this->_run.has_value() ? this->report(360) : Json::Value{};
    return json;
}

Json::Value master::report(size_t points) const
{
    auto& run           = this->_run.value();
    auto  json          = Json::Value{};
    json["id"]          = run.id;
    json["kind"]        = run.kind;
    json["state"]       = run.state;
    json["started_at"]  = run.started_at;
    json["finished_at"] = run.finished_at;
    json["elapsed"]     = std::round(run.elapsed);
    json["client"]      = run.client;
    json["target"]      = run.target;
    json["retry"]       = run.retry;
    json["users"]       = run.users;
    json["rate"]        = run.rate;
    json["cap"]         = run.cap;
    json["duration"]    = run.duration;
    json["offset"]      = run.offset;
    json["stats"]       = master::table(run.totals, run.elapsed);
    json["suites"]      = Json::Value{Json::arrayValue};
    for (auto& suite : run.suites)
    {
        auto  agent        = this->_agents.find(suite.agent);
        auto& item         = json["suites"].append(Json::Value{});
        item["name"]       = suite.name;
        item["serial"]     = suite.serial;
        item["state"]      = suite.state;
        item["agent"]      = suite.agent;
        item["agent_name"] = agent != this->_agents.end() ? agent->second.name : std::string{};
        item["attempts"]   = suite.attempts;
        item["seconds"]    = std::round(suite.seconds * 10.0) / 10.0;
        item["log"]        = suite.log;
    }

    json["timeline"] = Json::Value{Json::arrayValue};
    auto size        = static_cast<size_t>(run.timeline.size());
    for (auto i = size > points ? size - points : size_t{0}; i < size; i++)
    {
        json["timeline"].append(run.timeline[static_cast<Json::ArrayIndex>(i)]);
    }

    json["goals"]   = Json::Value{Json::objectValue};
    json["levels"]  = Json::Value{Json::objectValue};
    json["hunters"] = Json::Value{Json::arrayValue};
    json["hosts"]   = Json::Value{Json::objectValue};
    for (auto& [_, agent] : this->_agents)
    {
        if (agent.metrics["run_id"].asString() != run.id)
            continue;

        for (auto& hunter : points > 0 ? agent.metrics["hunters"] : Json::Value{Json::arrayValue})
        {
            json["hunters"].append(hunter);
        }

        for (auto& host : agent.metrics["hosts"].getMemberNames())
        {
            json["hosts"][host] = json["hosts"].get(host, 0).asUInt() + agent.metrics["hosts"][host].asUInt();
        }

        for (auto& goal : agent.metrics["goals"].getMemberNames())
        {
            json["goals"][goal] = json["goals"].get(goal, 0).asUInt() + agent.metrics["goals"][goal].asUInt();
        }

        for (auto& band : agent.metrics["levels"].getMemberNames())
        {
            json["levels"][band] = json["levels"].get(band, 0).asUInt() + agent.metrics["levels"][band].asUInt();
        }
    }
    return json;
}

Json::Value master::table(const std::map<std::string, stats::entry>& totals, double elapsed)
{
    auto rows = Json::Value{Json::arrayValue};
    for (auto& [name, entry] : totals)
    {
        auto& row       = rows.append(Json::Value{});
        row["name"]     = name;
        row["count"]    = static_cast<Json::UInt64>(entry.count);
        row["failures"] = static_cast<Json::UInt64>(entry.failures);
        row["timed"]    = static_cast<Json::UInt64>(entry.timed);
        row["avg"]      = entry.timed > 0 ? std::round(entry.sum / entry.timed) : 0.0;
        row["min"]      = std::round(entry.min);
        row["max"]      = std::round(entry.max);
        row["p50"]      = std::round(entry.percentile(0.50));
        row["p95"]      = std::round(entry.percentile(0.95));
        row["p99"]      = std::round(entry.percentile(0.99));
        row["rps"]      = elapsed > 0.0 ? std::round(entry.count / elapsed * 10.0) / 10.0 : 0.0;
    }
    return rows;
}

Json::Value master::suites() const
{
    auto json         = Json::Value{};
    json["revision"]  = this->_revision;
    json["loaded_at"] = this->_loaded_at;
    json["files"]     = this->_files;
    json["suites"]    = Json::Value{Json::arrayValue};
    for (auto& suite : this->_catalog)
    {
        auto& item         = json["suites"].append(Json::Value{});
        item["name"]       = suite.name;
        item["serial"]     = suite.serial;
        item["extra_slot"] = suite.extra_slot;
        item["bots"]       = suite.bots;
        item["error"]      = suite.error;
        item["labels"]     = Json::Value{Json::arrayValue};
        for (auto& label : suite.labels)
        {
            item["labels"].append(label);
        }
    }
    return json;
}

Json::Value master::targets() const
{
    auto targets = std::vector<Json::Value>{};
    for (auto path : {"config/targets.json", "config/targets.local.json"})
    {
        auto input = std::ifstream(path, std::ios::binary);
        auto text  = std::string{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        auto json  = Json::Value{};
        if (link::decode(text, json) == false || json.isObject() == false || json["targets"].isArray() == false)
            continue;

        for (auto& target : json["targets"])
        {
            auto found = std::find_if(targets.begin(), targets.end(), [&target](const auto& existing) {
                return existing["name"].asString() == target["name"].asString();
            });
            if (found != targets.end())
                *found = target;
            else
                targets.push_back(target);
        }
    }

    auto result       = Json::Value{};
    result["targets"] = Json::Value{Json::arrayValue};
    for (auto& target : targets)
    {
        result["targets"].append(target);
    }
    return result;
}

Json::Value master::history() const
{
    auto runs = std::vector<Json::Value>{};
    if (std::filesystem::exists("logs/runs"))
    {
        for (auto& directory : std::filesystem::directory_iterator("logs/runs"))
        {
            auto input   = std::ifstream(directory.path() / "summary.json", std::ios::binary);
            auto text    = std::string{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
            auto summary = Json::Value{};
            if (link::decode(text, summary) == false)
                continue;

            auto brief           = Json::Value{};
            brief["id"]          = summary["id"];
            brief["kind"]        = summary["kind"];
            brief["started_at"]  = summary["started_at"];
            brief["finished_at"] = summary["finished_at"];
            brief["elapsed"]     = summary["elapsed"];
            brief["users"]       = summary["users"];
            auto passed          = 0u;
            auto failed          = 0u;
            for (auto& suite : summary["suites"])
            {
                if (suite["state"].asString() == "passed")
                    passed++;
                else
                    failed++;
            }
            brief["passed"] = passed;
            brief["failed"] = failed;

            auto requests = uint64_t{0};
            auto failures = uint64_t{0};
            for (auto& row : summary["stats"])
            {
                auto name = row["name"].asString();
                if (name.starts_with("goal:") || name.starts_with("action:"))
                    continue;
                requests += row["count"].asUInt64();
                failures += row["failures"].asUInt64();
            }
            brief["requests"] = static_cast<Json::UInt64>(requests);
            brief["failures"] = static_cast<Json::UInt64>(failures);
            runs.push_back(brief);
        }
    }

    std::sort(runs.begin(), runs.end(), [](const auto& left, const auto& right) {
        return left["id"].asString() > right["id"].asString();
    });

    auto json = Json::Value{Json::arrayValue};
    for (size_t i = 0; i < std::min<size_t>(runs.size(), 100); i++)
    {
        json.append(runs.at(i));
    }
    return json;
}

Json::Value master::reload()
{
    auto result = Json::Value{};
    if (this->_run.has_value() && this->_run->state != "done")
    {
        result["error"] = "a run is in progress";
        return result;
    }

    auto catalog = std::vector<suite_entry>{};
    try
    {
        for (auto& script : fb::bot::integration::lua_integration_test::catalog())
        {
            auto suite       = suite_entry{};
            suite.name       = script.path.stem().string();
            suite.labels     = script.labels;
            suite.serial     = script.serial;
            suite.extra_slot = script.extra_slot;
            try
            {
                suite.bots = fb::bot::integration::lua_integration_test::peek_bot_count(script.path);
            }
            catch (const std::exception& e)
            {
                suite.error = e.what();
            }
            catalog.push_back(std::move(suite));
        }
    }
    catch (const std::exception& e)
    {
        result["error"] = e.what();
        return result;
    }

    auto files = Json::Value{Json::arrayValue};
    auto check = [&files](const std::string& path) {
        auto  input  = std::ifstream(path, std::ios::binary);
        auto  text   = std::string{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        auto  json   = Json::Value{};
        auto  reader = std::unique_ptr<Json::CharReader>(Json::CharReaderBuilder{}.newCharReader());
        auto  errors = std::string{};
        auto& item   = files.append(Json::Value{});
        item["path"] = path;
        item["size"] = static_cast<Json::UInt64>(text.size());
        if (input.is_open() == false)
            item["error"] = "missing";
        else if (reader->parse(text.data(), text.data() + text.size(), &json, &errors) == false)
            item["error"] = errors;
        return json;
    };

    auto data = check(fb::config<std::string>("load:data", "config/load.json"));
    check(data["spells"].get("catalog", "config/load-spells.json").asString());
    check(data["quests"].get("catalog", "config/load-quests.json").asString());
    check(fb::config<std::string>("load:messages", "config/load-messages.json"));

    this->_catalog   = std::move(catalog);
    this->_files     = files;
    this->_loaded_at = master::now("{:%Y-%m-%dT%H:%M:%SZ}");
    this->_revision++;
    this->broadcast();

    result["revision"] = this->_revision;
    result["suites"]   = static_cast<Json::UInt>(this->_catalog.size());
    result["files"]    = files;
    return result;
}

Json::Value master::start(const Json::Value& body)
{
    auto result = Json::Value{};
    if (this->_run.has_value() && this->_run->state != "done")
    {
        result["error"] = "a run is in progress";
        return result;
    }

    auto connected = std::any_of(this->_agents.begin(), this->_agents.end(), [](const auto& pair) {
        return pair.second.link != nullptr;
    });
    if (connected == false)
    {
        result["error"] = "no agent is connected";
        return result;
    }

    auto run       = run_state{};
    run.kind       = body["kind"].asString();
    run.id         = std::format("{}-{}", master::now("{:%Y%m%d-%H%M%S}"), run.kind);
    run.started_at = master::now("{:%Y-%m-%dT%H:%M:%SZ}");
    run.started    = clock::now();
    run.client     = body["client"].asString();
    run.target     = body["target"].asString();
    if (run.kind == "integration")
    {
        run.retry = body.get("retry", 1).asUInt();
        for (auto& name : body["suites"])
        {
            auto found = std::find_if(this->_catalog.begin(), this->_catalog.end(), [&name](const auto& suite) {
                return suite.name == name.asString();
            });
            if (found == this->_catalog.end())
            {
                result["error"] = std::format("unknown suite {}", name.asString());
                return result;
            }
            run.suites.push_back(suite_run{.name = found->name, .serial = found->serial});
        }

        if (run.suites.empty())
        {
            result["error"] = "no suite is selected";
            return result;
        }
    }
    else if (run.kind == "load")
    {
        run.users    = body["users"].asUInt();
        run.rate     = std::max(1u, body.get("rate", 10).asUInt());
        run.cap      = body.get("cap", 0).asUInt();
        run.duration = body.get("duration", 0).asUInt();
        run.offset   = body.get("offset", 0).asUInt();
        if (run.users == 0)
        {
            result["error"] = "users must be positive";
            return result;
        }
    }
    else
    {
        result["error"] = "unknown kind";
        return result;
    }

    for (auto& [_, agent] : this->_agents)
    {
        agent.excluded = false;
        agent.busy     = false;
        agent.users    = 0;
        agent.bots     = 0;
        agent.batch.clear();
    }

    this->_run = std::move(run);
    fb::console::puts("master: run {} started", this->_run->id);
    if (this->_run->kind == "load")
        this->rebalance();
    else
        this->dispatch();
    this->broadcast();

    result["id"] = this->_run->id;
    return result;
}

void master::stop()
{
    if (this->_run.has_value() == false || this->_run->state != "running")
        return;

    auto& run = this->_run.value();
    run.state = "stopping";
    for (auto& suite : run.suites)
    {
        if (suite.state == "pending")
            suite.state = "stopped";
    }

    for (auto& [_, agent] : this->_agents)
    {
        if (agent.busy && agent.link != nullptr)
        {
            auto stopped    = Json::Value{};
            stopped["type"] = "stop";
            agent.link->send(stopped);
        }
    }
    this->dispatch();
    this->broadcast();
}

void master::scale(uint32_t users)
{
    if (this->_run.has_value() == false || this->_run->kind != "load" || this->_run->state != "running" || users == 0)
        return;

    this->_run->users = users;
    this->rebalance();
    this->broadcast();
}

void master::dispatch()
{
    if (this->_run.has_value() == false || this->_run->state == "done")
        return;

    auto& run = this->_run.value();
    if (run.kind == "integration" && run.state == "running")
    {
        for (auto& [_, agent] : this->_agents)
        {
            if (agent.link == nullptr || agent.busy)
                continue;

            auto names = std::vector<std::string>{};
            for (auto& suite : run.suites)
            {
                if (names.size() >= agent.seats * 2)
                    break;
                if (suite.serial == false && suite.state == "pending")
                    names.push_back(suite.name);
            }

            if (names.empty())
                break;
            this->assign(agent, names);
        }

        auto open   = std::any_of(run.suites.begin(), run.suites.end(), [](const auto& suite) {
            return suite.serial == false &&
                   (suite.state == "pending" || suite.state == "assigned" || suite.state == "running");
        });
        auto serial = std::any_of(run.suites.begin(), run.suites.end(), [](const auto& suite) {
            return suite.serial && (suite.state == "assigned" || suite.state == "running");
        });
        auto next   = std::find_if(run.suites.begin(), run.suites.end(), [](const auto& suite) {
            return suite.serial && suite.state == "pending";
        });
        auto idle   = std::find_if(this->_agents.begin(), this->_agents.end(), [](const auto& pair) {
            return pair.second.link != nullptr && pair.second.busy == false;
        });
        if (open == false && serial == false && next != run.suites.end() && idle != this->_agents.end())
            this->assign(idle->second, {next->name});
    }

    auto busy = std::any_of(this->_agents.begin(), this->_agents.end(), [](const auto& pair) {
        return pair.second.busy;
    });
    auto open = std::any_of(run.suites.begin(), run.suites.end(), [](const auto& suite) {
        return suite.state == "pending" || suite.state == "assigned" || suite.state == "running";
    });
    if (busy)
        return;

    if (run.state == "stopping" || (run.kind == "integration" && open == false))
        this->finish();
}

void master::assign(agent_entry& agent, const std::vector<std::string>& names)
{
    auto& run            = this->_run.value();
    auto  lease          = this->_leases.find(agent.key);
    auto  message        = Json::Value{};
    message["type"]      = "run_suites";
    message["run_id"]    = run.id;
    message["client"]    = run.client;
    message["target"]    = run.target;
    message["seat_base"] = lease != this->_leases.end() ? lease->second.seat_base : 0;
    message["suites"]    = Json::Value{Json::arrayValue};
    for (auto& name : names)
    {
        message["suites"].append(name);
        for (auto& suite : run.suites)
        {
            if (suite.name == name && suite.state == "pending")
            {
                suite.state = "assigned";
                suite.agent = agent.id;
            }
        }
    }

    agent.busy  = true;
    agent.batch = names;
    agent.link->send(message);
}

void master::rebalance()
{
    if (this->_run.has_value() == false || this->_run->kind != "load" || this->_run->state != "running")
        return;

    auto& run      = this->_run.value();
    auto  eligible = std::vector<agent_entry*>{};
    auto  weight   = uint64_t{0};
    for (auto& [_, agent] : this->_agents)
    {
        if (agent.link == nullptr || agent.excluded)
            continue;

        eligible.push_back(&agent);
        weight += std::max(1u, agent.max_bots);
    }

    if (eligible.empty())
        return;

    auto limit = [&run](const agent_entry* agent) {
        auto capacity = std::max(1u, agent->max_bots);
        return run.cap > 0 ? std::min(capacity, run.cap) : capacity;
    };

    auto shares = std::vector<uint32_t>(eligible.size(), 0);
    auto given  = uint32_t{0};
    for (size_t i = 0; i < eligible.size(); i++)
    {
        auto share    = static_cast<uint32_t>(uint64_t{run.users} * std::max(1u, eligible.at(i)->max_bots) / weight);
        shares.at(i)  = std::min(share, limit(eligible.at(i)));
        given        += shares.at(i);
    }

    auto order = std::vector<size_t>(eligible.size());
    for (size_t i = 0; i < order.size(); i++)
    {
        order.at(i) = i;
    }
    std::sort(order.begin(), order.end(), [&eligible](size_t left, size_t right) {
        return eligible.at(left)->max_bots > eligible.at(right)->max_bots;
    });

    auto progressed = true;
    while (given < run.users && progressed)
    {
        progressed = false;
        for (auto i : order)
        {
            if (given >= run.users)
                break;
            if (shares.at(i) >= limit(eligible.at(i)))
                continue;

            shares.at(i)++;
            given++;
            progressed = true;
        }
    }

    for (size_t i = 0; i < eligible.size(); i++)
    {
        auto& agent = *eligible.at(i);
        auto  share = shares.at(i);
        if (agent.busy)
        {
            if (agent.users != share)
            {
                auto scaled     = Json::Value{};
                scaled["type"]  = "scale";
                scaled["users"] = share;
                agent.link->send(scaled);
                agent.users = share;
            }
        }
        else if (share > 0)
        {
            auto lease          = this->_leases.find(agent.key);
            auto pace           = static_cast<double>(run.rate) * share / run.users;
            auto message        = Json::Value{};
            message["type"]     = "run_load";
            message["run_id"]   = run.id;
            message["users"]    = share;
            message["rate"]     = std::max(1u, static_cast<uint32_t>(std::lround(pace)));
            message["interval"] = pace >= 1.0 ? 1000u : static_cast<uint32_t>(std::lround(1000.0 / pace));
            message["process"]  = lease != this->_leases.end() ? lease->second.process : 0;
            message["offset"]   = run.offset;
            message["client"]   = run.client;
            message["target"]   = run.target;
            agent.link->send(message);
            agent.busy  = true;
            agent.users = share;
        }
    }
}

void master::finish()
{
    auto& run       = this->_run.value();
    run.state       = "done";
    run.finished_at = master::now("{:%Y-%m-%dT%H:%M:%SZ}");
    run.elapsed     = std::chrono::duration<double>(clock::now() - run.started).count();
    for (auto& [_, agent] : this->_agents)
    {
        agent.busy     = false;
        agent.excluded = false;
        agent.users    = 0;
        agent.batch.clear();
    }

    auto directory = std::filesystem::path{"logs/runs"} / run.id;
    std::filesystem::create_directories(directory);

    auto summary = this->report(0);
    summary.removeMember("timeline");
    auto writer           = Json::StreamWriterBuilder{};
    writer["indentation"] = "  ";
    writer["emitUTF8"]    = true;
    std::ofstream(directory / "summary.json", std::ios::binary) << Json::writeString(writer, summary);

    auto timeline = std::ofstream(directory / "timeline.jsonl", std::ios::binary);
    for (auto& point : run.timeline)
    {
        timeline << link::serialize(point) << '\n';
    }

    auto csv = std::ofstream(directory / "stats.csv", std::ios::binary);
    csv << "name,count,failures,avg_ms,min_ms,max_ms,p50_ms,p95_ms,p99_ms,rps\n";
    for (auto& row : summary["stats"])
    {
        csv << std::format("{},{},{},{},{},{},{},{},{},{}\n",
                           row["name"].asString(),
                           row["count"].asUInt64(),
                           row["failures"].asUInt64(),
                           row["avg"].asDouble(),
                           row["min"].asDouble(),
                           row["max"].asDouble(),
                           row["p50"].asDouble(),
                           row["p95"].asDouble(),
                           row["p99"].asDouble(),
                           row["rps"].asDouble());
    }

    fb::console::puts("master: run {} finished", run.id);
    this->broadcast();
}

void master::handle(agent_entry& agent, const Json::Value& message)
{
    auto type    = message["type"].asString();
    auto current = this->_run.has_value() && message["run_id"].asString() == this->_run->id;
    if (type == "stats")
    {
        agent.bots = message["bots"].asUInt();
        if (current == false)
            return;

        for (auto& name : message["entries"].getMemberNames())
        {
            this->_run->totals[name].merge(message["entries"][name]);
            this->_run->window[name].merge(message["entries"][name]);
        }
    }
    else if (type == "metrics")
    {
        if (current)
            agent.metrics = message;
    }
    else if (type == "sync")
    {
        if (current == false)
            return;

        auto relayed      = Json::Value{};
        relayed["type"]   = "sync";
        relayed["events"] = message["events"];
        for (auto& [id, other] : this->_agents)
        {
            if (id != agent.id && other.busy && other.link != nullptr)
                other.link->send(relayed);
        }
    }
    else if (type == "suite_started" || type == "suite_finished")
    {
        if (current == false)
            return;

        auto name = message["name"].asString();
        for (auto& suite : this->_run->suites)
        {
            if (suite.name != name || suite.agent != agent.id ||
                (suite.state != "assigned" && suite.state != "running"))
                continue;

            if (type == "suite_started")
            {
                suite.state = "running";
            }
            else
            {
                suite.state   = message["success"].asBool() ? "passed" : "failed";
                suite.seconds = message["seconds"].asDouble();
                suite.log     = message["log"].asString();
            }
            break;
        }
    }
    else if (type == "child_exit")
    {
        if (agent.busy == false || current == false)
            return;

        auto& run  = this->_run.value();
        agent.busy = false;
        agent.bots = 0;
        if (run.kind == "integration")
        {
            for (auto& suite : run.suites)
            {
                if (suite.agent != agent.id || (suite.state != "assigned" && suite.state != "running"))
                    continue;

                suite.state = "failed";
                suite.log =
                    std::format("{}\nprocess exited with code {}", message["log"].asString(), message["code"].asInt());
            }
            agent.batch.clear();
            this->dispatch();
        }
        else
        {
            agent.users = 0;
            if (run.state == "running")
            {
                agent.excluded = true;
                fb::console::puts("master: agent #{} load process exited ({})", agent.id, message["code"].asInt());
                this->rebalance();
            }
            this->dispatch();
        }
    }
}

void master::drop(agent_entry& agent)
{
    fb::console::puts("master: agent #{} {} disconnected", agent.id, agent.key);
    agent.link.reset();
    agent.lost = clock::now();
    agent.bots = 0;
    if (agent.busy && this->_run.has_value())
    {
        auto& run = this->_run.value();
        for (auto& suite : run.suites)
        {
            if (suite.agent != agent.id || (suite.state != "assigned" && suite.state != "running"))
                continue;

            suite.attempts++;
            suite.state = run.state == "running" && suite.attempts <= run.retry ? "pending" : "lost";
            if (suite.state == "pending")
                suite.agent = 0;
        }
    }

    agent.busy  = false;
    agent.users = 0;
    agent.batch.clear();
    this->rebalance();
    this->dispatch();
    this->broadcast();
}

void master::broadcast()
{
    std::erase_if(this->_dashboards, [](const auto& link) {
        return link->closed();
    });

    if (this->_dashboards.empty())
        return;

    auto state = this->snapshot();
    for (auto& link : this->_dashboards)
    {
        link->send(state);
    }
}

void master::save_leases() const
{
    auto json = Json::Value{Json::objectValue};
    for (auto& [key, entry] : this->_leases)
    {
        json[key]["process"]   = entry.process;
        json[key]["seat_base"] = entry.seat_base;
        json[key]["seats"]     = entry.seats;
    }

    std::filesystem::create_directories("logs");
    std::ofstream("logs/cluster-leases.json", std::ios::binary) << link::serialize(json);
}

void master::load_leases()
{
    auto input = std::ifstream("logs/cluster-leases.json", std::ios::binary);
    auto text  = std::string{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    auto json  = Json::Value{};
    if (link::decode(text, json) == false)
        return;

    for (auto& key : json.getMemberNames())
    {
        this->_leases[key] = lease{
            .process   = json[key]["process"].asUInt(),
            .seat_base = json[key]["seat_base"].asUInt(),
            .seats     = json[key]["seats"].asUInt(),
        };
    }
}

std::string master::now(std::string_view format)
{
    auto time = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    return std::vformat(format, std::make_format_args(time));
}
