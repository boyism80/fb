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

    /**
     * @brief      Handles game time updates from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The time response containing current game time.
     *
     * @return     An async task that completes when time processing is finished.
     */
    async::task<void> handle_time(game_bot& bot, const fb::protocol::game::response::time& response);

    /**
     * @brief      Handles character state updates from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The internal state update response.
     *
     * @return     An async task that completes when state processing is finished.
     */
    async::task<void> handle_state(game_bot& bot, const fb::protocol::game::response::update_internal& response);

    /**
     * @brief      Handles game option updates from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The option update response.
     *
     * @return     An async task that completes when option processing is finished.
     */
    async::task<void> handle_option(game_bot& bot, const fb::protocol::game::response::option& response);

    /**
     * @brief      Handles message notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The message response containing notification text.
     *
     * @return     An async task that completes when message processing is finished.
     */
    async::task<void> handle_message(game_bot& bot, const fb::protocol::game::response::message& response);

    /**
     * @brief      Handles sequence ID updates from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The ID response containing the new sequence number.
     *
     * @return     An async task that completes when sequence processing is finished.
     */
    async::task<void> handle_sequence(game_bot& bot, const fb::protocol::game::response::id& response);

    /**
     * @brief      Handles spell updates from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The spell update response.
     *
     * @return     An async task that completes when spell processing is finished.
     */
    async::task<void> handle_spell_update(game_bot& bot, const fb::protocol::game::response::spell_update& response);

    /**
     * @brief      Handles chat messages from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The chat response containing message data.
     *
     * @return     An async task that completes when chat processing is finished.
     */
    async::task<void> handle_chat(game_bot& bot, const fb::protocol::game::response::chat& response);

    /**
     * @brief      Handles action notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The action response containing action data.
     *
     * @return     An async task that completes when action processing is finished.
     */
    async::task<void> handle_action(game_bot& bot, const fb::protocol::game::response::action& response);

    /**
     * @brief      Handles direction change notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The direction response containing new facing direction.
     *
     * @return     An async task that completes when direction processing is finished.
     */
    async::task<void> handle_direction(game_bot& bot, const fb::protocol::game::response::direction& response);

    /**
     * @brief      Handles position updates from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The position response containing current position.
     *
     * @return     An async task that completes when position processing is finished.
     */
    async::task<void> handle_position(game_bot& bot, const fb::protocol::game::response::position& response);

    /**
     * @brief      Handles movement notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The move response containing movement data.
     *
     * @return     An async task that completes when movement processing is finished.
     */
    async::task<void> handle_move(game_bot& bot, const fb::protocol::game::response::move& response);

    /**
     * @brief      Handles sound effect notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The sound response containing sound effect data.
     *
     * @return     An async task that completes when sound processing is finished.
     */
    async::task<void> handle_sound(game_bot& bot, const fb::protocol::game::response::sound& response);

    /**
     * @brief      Handles visual effect notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The effect response containing effect data.
     *
     * @return     An async task that completes when effect processing is finished.
     */
    async::task<void> handle_effect(game_bot& bot, const fb::protocol::game::response::effect& response);

    /**
     * @brief      Handles object hide notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The hide response containing hide data.
     *
     * @return     An async task that completes when hide processing is finished.
     */
    async::task<void> handle_hide(game_bot& bot, const fb::protocol::game::response::hide& response);

    /**
     * @brief      Handles death notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The die response containing death data.
     *
     * @return     An async task that completes when death processing is finished.
     */
    async::task<void> handle_die(game_bot& bot, const fb::protocol::game::response::die& response);

    /**
     * @brief      Handles buff application notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The buff response containing buff data.
     *
     * @return     An async task that completes when buff processing is finished.
     */
    async::task<void> handle_buff(game_bot& bot, const fb::protocol::game::response::spell_buff& response);

    /**
     * @brief      Handles buff removal notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The unbuff response containing buff removal data.
     *
     * @return     An async task that completes when unbuff processing is finished.
     */
    async::task<void> handle_unbuff(game_bot& bot, const fb::protocol::game::response::spell_unbuff& response);

    /**
     * @brief      Handles object update notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The update response containing object data.
     *
     * @return     An async task that completes when update processing is finished.
     */
    async::task<void> handle_update(game_bot& bot, const fb::protocol::game::response::update& response);

    /**
     * @brief      Handles map configuration from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The map config response.
     *
     * @return     An async task that completes when map processing is finished.
     */
    async::task<void> handle_map(game_bot& bot, const fb::protocol::game::response::map_config& response);

    /**
     * @brief      Handles server transfer notifications.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The transfer response containing new server information.
     *
     * @return     An async task that completes when transfer processing is finished.
     */
    async::task<void> handle_transfer(game_bot& bot, const fb::protocol::response::transfer& response);
};

} // namespace fb::bot::load

#endif