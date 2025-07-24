#ifndef __BOT_GAME_CONTROLLER_H__
#define __BOT_GAME_CONTROLLER_H__

#include <fb/bot/controller.h>
#include <fb/game/protocol.h>
#include <fb/bot/game_bot.h>

namespace fb::bot {

/**
 * @brief      Abstract base class for game bot bot_controllers.
 *
 *             This class defines the common interface for all game bot bot_controllers.
 *             It provides the foundation for both load testing and integration testing
 *             bot_controllers while maintaining a consistent interface for bot management.
 */
class game_bot_controller : public bot_controller<game_bot>
{
public:
    using bot_type = game_bot; ///< Type alias for the managed bot type

public:
    struct transfer_context
    {
        using promise_type = async::task_completion_source<std::shared_ptr<game_bot>>;

        std::shared_ptr<promise_type>      promise;
        std::shared_ptr<fb::timer>         timer;
        std::string                        name;
        std::atomic<bool>                  completed;
        std::weak_ptr<game_bot_controller> controller_weak;

        transfer_context(std::shared_ptr<promise_type>      promise,
                         std::string                        name,
                         std::weak_ptr<game_bot_controller> controller) :
            promise(promise),
            name(name),
            controller_weak(controller)
        { }

        void complete_success(std::shared_ptr<game_bot> bot)
        {
            if (completed.exchange(true))
                return; // Already completed

            if (timer)
                timer->cancel();
            promise->set_value(bot);
        }

        void complete_timeout()
        {
            if (completed.exchange(true))
                return; // Already completed

            // Remove hook from bot (if bot still exists)
            if (auto controller = controller_weak.lock())
                controller->remove_transfer_context(name);

            promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
        }

        async::task<std::shared_ptr<game_bot>> task()
        {
            return promise->task();
        }
    };

private:
    std::unordered_map<std::string, std::shared_ptr<transfer_context>> _transfer_contexts;

protected:
    /**
     * @brief      Protected constructor to enforce abstract nature of this class.
     *
     *             Only derived classes can instantiate this bot_controller.
     *
     * @param      container  The bot container that manages this bot_controller.
     */
    game_bot_controller(bot_container& container);

public:
    /**
     * @brief      Virtual destructor for proper cleanup in derived classes.
     */
    virtual ~game_bot_controller() = default;

    /**
     * @brief      Initializes the game bot_controller and sets up timers.
     *
     *             This method must be implemented by derived classes to provide
     *             specific initialization logic for different test modes.
     */
    virtual void initialize() = 0;

public:
    /**
     * @brief      Handles game bot connection events.
     *
     *             This method is called when a bot successfully connects to the game server.
     *             Derived classes should implement specific connection handling logic.
     *
     * @param      bot  The game bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(game_bot& bot) override = 0;

    /**
     * @brief      Handles game bot disconnection events.
     *
     *             This method is called when a bot disconnects from the game server.
     *             Derived classes should implement specific disconnection handling logic.
     *
     * @param      bot  The game bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(game_bot& bot) override;

    /**
     * @brief      Determines decryption policy for protocol messages.
     *
     *             Returns whether a protocol message should be decrypted
     *             based on the command type. Some messages (like transfers)
     *             are not encrypted and should bypass decryption.
     *
     * @param[in]  cmd  The protocol command identifier.
     *
     * @return     True if the message should be decrypted, false otherwise.
     */
    bool decrypt_policy(int cmd) const override final;

private:
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
     * @brief      Handles map configuration from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The map config response.
     *
     * @return     An async task that completes when map processing is finished.
     */
    async::task<void> handle_map_config(game_bot& bot, const fb::protocol::game::response::map_config& response);

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
     * @brief      Handles oid ID updates from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The ID response containing the new oid number.
     *
     * @return     An async task that completes when oid processing is finished.
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
     * @brief      Handles spell removal from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The spell remove response.
     *
     * @return     An async task that completes when spell removal processing is finished.
     */
    async::task<void> handle_spell_remove(game_bot& bot, const fb::protocol::game::response::spell_remove& response);

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

    /**
     * @brief   Updates bot's character appearance data from update_external response
     *
     *          This method handles the common character data that is shared between
     *          detailed and brief update_external responses. Only updates the bot's
     *          data if the oid matches the bot's own oid.
     *
     * @tparam  Detailed  Template parameter indicating if this is detailed or brief update
     * @param   bot       The bot instance to update
     * @param   response  The update_external response containing character data
     */
    template <bool Detailed>
    async::task<void> handle_update_external(game_bot&                                                      bot,
                                             const fb::protocol::game::response::update_external<Detailed>& response)
    {
        // Only update if this is our bot's character data
        if (bot.oid() != response.oid)
            co_return;

        // Update position data (only available in detailed mode)
        if constexpr (Detailed)
        {
            bot.set_position(point<uint16_t>(response.x, response.y));
            bot.set_direction(static_cast<DIRECTION>(response.direction));
        }

        // Update common appearance data
        bot.set_disguised(response.disguised);
        bot.set_sex(response.sex);
        bot.set_state(response.state);
        bot.set_look(response.look);
        bot.set_color(response.color);

        // Update equipment appearance
        bot.set_armor_dress(response.armor_dress);
        bot.set_armor_color(response.armor_color);
        bot.set_weapon_dress(response.weapon_dress);
        bot.set_weapon_color(response.weapon_color);
        bot.set_shield_dress(response.shield_dress);
        bot.set_shield_color(response.shield_color);

        // Update additional data
        bot.set_head_marker(response.head_marker);
        bot.set_name(response.name);
        co_return;
    }

private:
    /**
     * @brief      Handles item acquisition and update notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The item update response containing item data.
     *
     * @return     An async task that completes when item update processing is finished.
     */
    async::task<void> handle_item_update(game_bot& bot, const fb::protocol::game::response::item_update& response);

    /**
     * @brief      Handles item removal notifications from the server.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The item remove response containing removal data.
     *
     * @return     An async task that completes when item removal processing is finished.
     */
    async::task<void> handle_item_remove(game_bot& bot, const fb::protocol::game::response::item_remove& response);

    /**
     * @brief      Handles internal info notifications from the server.
     *
     *             This method handles the internal info response from the server.
     *             It updates the bot's internal info data with the new values.
     *
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The internal info response containing internal info data.
     *
     * @return     An async task that completes when internal info processing is finished.
     */
    async::task<void> handle_internal_info(game_bot& bot, const fb::protocol::game::response::internal_info& response);

public:
    bool register_transfer_context(const fb::protocol::header& protocol, std::shared_ptr<transfer_context> context);
    void remove_transfer_context(std::string name);
    bool invoke_transfer_context(std::string name, std::shared_ptr<game_bot> bot);
};

} // namespace fb::bot

#endif