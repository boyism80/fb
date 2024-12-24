#include <bot.h>

using namespace fb::bot;

bot_container::bot_container(boost::asio::io_context& context) :
    _context(context),
    fb::context(context, "BOT", fb::config<uint32_t>("thread:logic")),
    _remained_count(fb::config<uint32_t>("spawn_count") / fb::config<uint32_t>("io_size"))
{
    for (int i = 0; i < this->threads.count(); i++)
    {
        auto thread = this->threads.at(i);
        thread->dispatch([](auto& thread) -> async::task<void> {
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

void bot_container::remove(base_bot& bot)
{
    auto id     = bot.id % this->threads.size();
    auto thread = this->threads[id];
    auto fn     = [this, id = bot.id](auto& thread) -> async::task<void> {
        auto params = thread.data<bot_thread_params>();
        params->bots.erase(id);
        co_return;
    };

    std::ignore = thread->dispatch(fn);
}

async::task<void> bot_container::handle_bot_spawn()
{
    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(fb::config<std::string>("ip")),
                                                   fb::config<uint16_t>("port"));
    auto count    = std::min(this->_remained_count, fb::config<uint32_t>("spawn_per_interval"));
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