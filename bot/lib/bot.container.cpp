#include <fb/bot/bot.h>

using namespace fb::bot;

bot_container::bot_container(boost::asio::io_context& context) :
    _context(context),
    _threads(context, 1)
{
    this->_threads.settimer(std::bind(&bot_container::handle_timer, this, std::placeholders::_1, std::placeholders::_2), 100ms);
}

bot_container::~bot_container()
{ }

boost::asio::io_context& bot_container::context() const
{
    return this->_context;
}

void bot_container::remove(base_bot& bot)
{
    auto id     = bot.id % this->_threads.size();
    auto thread = this->_threads[id];
    auto fn     = [this, &bot]() -> async::task<void> {
        auto i = this->_bots.find(bot.id);
        if (i == this->_bots.end())
            co_return;

        delete i->second;
        this->_bots.erase(i);
    };

    thread->dispatch(fn);
}

void bot_container::handle_timer(std::chrono::steady_clock::duration now, std::thread::id id)
{
    for (auto& [k, v] : this->_bots)
    {
        v->on_timer(now);
    }
}

async::task<void> bot_container::dispatch(uint32_t id, std::function<async::task<void>()>&& fn)
{
    auto index  = id % this->_threads.size();
    auto thread = this->_threads[index];
    co_await thread->dispatch(fn);
}