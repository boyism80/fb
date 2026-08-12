#include <fb/bot/load/game_controller.h>
#include <fb/bot/game_bot.h>

using namespace fb::bot::load;

game_bot_controller::game_bot_controller(bot_container& container) :
    fb::bot::game_bot_controller(container)
{ }

void game_bot_controller::initialize()
{
    this->bind_thread_timer(&game_bot_controller::on_timer, 100ms);
}

async::task<void> game_bot_controller::on_timer(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->container.threads.at(id);
    auto params = thread->data<bot_thread_params>();
    if (params == nullptr)
        co_return;

    auto guard = this->_bots.enter_read();
    for (auto& [_, bot] : params->bots)
    {
        if (guard.value().contains(bot->id) == false)
            continue;

        auto typed_bot = static_cast<game_bot*>(bot.get());
        typed_bot->process_random_pattern(now);
    }
    co_return;
}

async::task<void> game_bot_controller::on_bot_connected(game_bot& bot)
{
    // Send game login packet with transfer buffer.
    // Bootstrap 0x10 always uses the v550 layout; the packed client_version field
    // inside the transfer blob still establishes the session version.
    using login_request = fb::protocol::game::request::login<fb::protocol::CLIENT_VERSION::v550>;
    bot.send(login_request(bot.transfer_buffer()), false, true);

    // Bot is now managed by bot_controller's thread-safe collection
    co_return;
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    co_await super::on_bot_disconnected(bot);
    // Bot is automatically removed from bot_controller's thread-safe collection
    co_return;
}