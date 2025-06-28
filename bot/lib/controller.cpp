#include <fb/bot/container.h>
#include <fb/bot/gateway_bot.h>
#include <fb/bot/login_bot.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/gateway_controller.h>
#include <fb/bot/login_controller.h>
#include <fb/bot/game_controller.h>

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

void bot_container::set_gateway_bot_controller(std::unique_ptr<gateway_bot_controller> bot_controller)
{
    this->gateway = std::move(bot_controller);
}

void bot_container::set_login_bot_controller(std::unique_ptr<login_bot_controller> bot_controller)
{
    this->login = std::move(bot_controller);
}

void bot_container::set_game_bot_controller(std::unique_ptr<game_bot_controller> bot_controller)
{
    this->game = std::move(bot_controller);
}

void bot_container::initialize()
{
    this->_running = true;
    // Timer setup moved to individual bot_controllers
    if (this->gateway)
        this->gateway->initialize();
    if (this->login)
        this->login->initialize();
    if (this->game)
        this->game->initialize();
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
