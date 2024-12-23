#include <bot.h>

using namespace fb::bot;

bot_container::bot_container(boost::asio::io_context& context) :
    _context(context),
    fb::context(context, "BOT", 1)
{
    this->threads.settimer(std::bind(&bot_container::handle_timer, this, std::placeholders::_1, std::placeholders::_2),
                           100ms);
}

bot_container::~bot_container()
{ }

boost::asio::io_context& bot_container::context() const
{
    return this->_context;
}

void bot_container::remove(base_bot& bot)
{
    auto id     = bot.id % this->threads.size();
    auto thread = this->threads[id];
    auto fn     = [this, &bot](auto& thread) -> async::task<void> {
        auto i = this->_bots.find(bot.id);
        if (i == this->_bots.end())
            co_return;

        delete i->second;
        this->_bots.erase(i);
    };

    std::ignore = thread->dispatch(fn);
}

async::task<void> bot_container::handle_timer(const fb::model::datetime& now, std::thread::id id)
{
    for (auto& [k, v] : this->_bots)
    {
        co_await v->on_timer(now);
    }
}

async::task<void> bot_container::dispatch(uint32_t id, std::function<async::task<void>(fb::thread&)>&& fn)
{
    auto index  = id % this->threads.size();
    auto thread = this->threads[index];
    co_await thread->dispatch(fn);
}