#ifndef __BOT_GAME_LOAD_CONTROLLER_H__
#define __BOT_GAME_LOAD_CONTROLLER_H__

#include <fb/bot/game_controller.h>
#include <fb/game/protocol.h>
#include <fb/bot/game_bot.h>

namespace fb::bot::load {

class game_bot_controller : public fb::bot::game_bot_controller
{
private:
    using super = fb::bot::game_bot_controller;

public:
    using bot_type = game_bot; ///< Type alias for the managed bot type

    game_bot_controller(bot_container& container);

    void initialize();

public:
    virtual async::task<void> on_bot_connected(game_bot& bot) override;
    virtual async::task<void> on_bot_disconnected(game_bot& bot) override;

private:
    async::task<void> on_timer(const fb::model::datetime& now, std::thread::id id);
};

} // namespace fb::bot::load

#endif