#include <fb/bot/load/game_controller.h>
#include <fb/bot/game_bot.h>

using namespace fb::bot::load;

game_bot_controller::game_bot_controller(bot_container& container) :
    fb::bot::game_bot_controller(container)
{ }

void game_bot_controller::initialize()
{
    this->bind_thread_timer(&game_bot_controller::handle_timer, 100ms);
}

async::task<void> game_bot_controller::handle_timer(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->container.threads.at(id);
    auto params = thread->data<bot_thread_params>();
    if (params == nullptr)
        co_return;

    this->_bots.read([&](const auto& bots) {
        for (auto& [_, bot] : params->bots)
        {
            if (bots.contains(bot->id) == false)
                continue;

            auto typed_bot = static_cast<game_bot*>(bot.get());
            typed_bot->process_random_pattern(now);
        }
    });
    co_return;
}

async::task<void> game_bot_controller::on_bot_connected(game_bot& bot)
{
    // Send game login packet with transfer buffer
    bot.send(fb::protocol::game::request::login(bot.transfer_buffer()), false, true);

    // Bot is now managed by bot_controller's thread-safe collection
    co_return;
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    co_await super::on_bot_disconnected(bot);
    // Bot is automatically removed from bot_controller's thread-safe collection
    co_return;
}