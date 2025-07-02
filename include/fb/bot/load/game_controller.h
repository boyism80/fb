#ifndef __BOT_GAME_LOAD_CONTROLLER_H__
#define __BOT_GAME_LOAD_CONTROLLER_H__

#include <fb/bot/game_controller.h>
#include <fb/game/protocol.h>
#include <fb/bot/game_bot.h>

namespace fb::bot::load {

class game_bot_controller : public fb::bot::game_bot_controller
{
public:
    using bot_type = game_bot; ///< Type alias for the managed bot type

    game_bot_controller(bot_container& container);

    /**
     * @brief      Initializes the game bot_controller and sets up timers.
     */
    void initialize();

public:
    /**
     * @brief      Handles game bot connection events.
     *
     *             Increments the game bot counter and sends login packet when a bot connects.
     *
     * @param      bot  The game bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(game_bot& bot) override;

    /**
     * @brief      Handles game bot disconnection events.
     *
     *             Decrements the game bot counter when a bot disconnects.
     *
     * @param      bot  The game bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(game_bot& bot) override;

private:
    /**
     * @brief      Handles timer events for all bots in a specific thread.
     *
     *             Distributes timer notifications to all bot instances
     *             managed by the specified thread.
     *
     * @param[in]  now  The current date and time.
     * @param[in]  id   The thread identifier.
     *
     * @return     An async task that completes when all bots have processed the timer.
     */
    async::task<void> handle_timer(const fb::model::datetime& now, std::thread::id id);
};

} // namespace fb::bot::load

#endif