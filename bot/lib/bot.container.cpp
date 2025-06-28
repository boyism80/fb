#include <fb/bot/bot.container.h>
#include <fb/bot/bot.gateway.h>
#include <fb/bot/bot.login.h>
#include <fb/bot/bot.game.h>
#include <fb/bot/bot.gateway.controller.h>
#include <fb/bot/bot.login.controller.h>
#include <fb/bot/bot.game.controller.h>

using namespace fb::bot;

bot_container::bot_container(boost::asio::io_context& context) :
    _context(context),
    fb::context(context, "BOT", fb::config<uint32_t>("thread:logic"))
{
    for (int i = 0; i < this->threads.count(); i++)
    {
        auto thread = this->threads.at(i);
        std::ignore = thread->dispatch([](auto& thread) -> async::task<void> {
            thread.data(new bot_thread_params{});
            co_return;
        });
    }
}

void bot_container::initialize()
{
    this->_running = true;
    // Timer setup moved to individual controllers
    this->gateway_controller->initialize();
    this->game_controller->initialize();
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

async::task<void> bot_container::dispatch(uint32_t id, std::function<async::task<void>(fb::thread&)>&& fn)
{
    auto index  = id % this->threads.size();
    auto thread = this->threads[index];
    co_await thread->dispatch(fn);
}
