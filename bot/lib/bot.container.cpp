#include <fb/bot/bot.h>

using namespace fb::bot;

bot_container::bot_container(boost::asio::io_context& context) : _context(context), _threads(context, std::thread::hardware_concurrency())
{
    this->_threads.settimer(std::bind(&bot_container::handle_timer, this, std::placeholders::_1, std::placeholders::_2), 100ms);
}

bot_container::~bot_container()
{
    
}

boost::asio::io_context& bot_container::context() const
{
    return this->_context;
}

void bot_container::remove(base_bot& bot)
{   MUTEX_GUARD(this->_bots_lock)

    auto id = bot.id % this->_threads.size();
    auto thread = this->_threads[id];
    auto fn = [this, &bot] (auto _)
    {
        auto i = this->_bots.find(bot.id);
        if (i == this->_bots.end())
            return;

        delete i->second;
        this->_bots.erase(i);
    };

    thread->dispatch(fn);
}

void bot_container::handle_timer(std::chrono::steady_clock::duration now, std::thread::id id)
{   MUTEX_GUARD(this->_bots_lock)

    for (auto& [k, v] : this->_bots)
    {
        auto index = v->id % this->_threads.size();
        auto thread = this->_threads[index];
        if (thread == nullptr)
            continue;

        if (thread->id() != id)
            continue;

        v->on_timer(now);
    }
}