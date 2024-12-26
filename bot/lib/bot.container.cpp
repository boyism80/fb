#include <bot.container.h>
#include <bot.gateway.h>
#include <bot.login.h>
#include <bot.game.h>

using namespace fb::bot;

bot_container::bot_container(boost::asio::io_context& context) :
    _context(context),
    fb::context(context, "BOT", fb::config<uint32_t>("thread:logic")),
    _remained_count(fb::config<uint32_t>("spawn_count") / fb::config<uint32_t>("io_size"))
{
    for (int i = 0; i < this->threads.count(); i++)
    {
        auto thread = this->threads.at(i);
        std::ignore = thread->dispatch([](auto& thread) -> async::task<void> {
            thread.data(new bot_thread_params{});
            co_return;
        });
    }

    this->bind_thread_timer(&bot_container::handle_timer, 100ms);
    this->bind_timer(&bot_container::handle_bot_spawn, std::chrono::milliseconds(fb::config<uint32_t>("interval")));
}

bot_container::~bot_container()
{
    for (int i = 0; i < this->threads.count(); i++)
    {
        auto thread = this->threads.at(i);
        delete thread->data<bot_thread_params>();
    }
}

boost::asio::io_context& bot_container::context() const
{
    return this->_context;
}

async::task<void> bot_container::handle_bot_spawn()
{
    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(fb::config<std::string>("ip")),
                                                   fb::config<uint16_t>("port"));
    auto count =
        std::min(this->_remained_count, fb::config<uint32_t>("spawn_per_interval") / fb::config<uint32_t>("io_size"));
    for (uint32_t i = 0; i < count; i++)
    {
        auto bot = this->create<fb::bot::gateway_bot>();
        bot->connect(endpoint);
    }

    this->_remained_count -= count;
    co_return;
}

async::task<void> bot_container::handle_timer(const datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->data<bot_thread_params>();

    for (auto& [_, bot] : params->bots)
    {
        bot->on_timer(now);
    }
    co_return;
}

async::task<void> bot_container::dispatch(uint32_t id, std::function<async::task<void>(fb::thread&)>&& fn)
{
    auto index  = id % this->threads.size();
    auto thread = this->threads[index];
    co_await thread->dispatch(fn);
}

void bot_container::display_spawned_bots()
{
    auto _1 = std::shared_lock<std::shared_mutex>(gateway_bot::_mutex);
    auto _2 = std::shared_lock<std::shared_mutex>(login_bot::_mutex);
    auto _3 = std::shared_lock<std::shared_mutex>(game_bot::_mutex);

    static auto y = fb::console::position();
    fb::console::position(y);
    fb::console::put("gateway\t\t{}", gateway_bot::_count);
    fb::console::position(y + 1);
    fb::console::put("login\t\t{}", login_bot::_count);
    fb::console::position(y + 2);
    fb::console::put("game\t\t{}", game_bot::_count);
}