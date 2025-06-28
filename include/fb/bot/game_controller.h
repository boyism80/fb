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
    virtual async::task<void> on_bot_disconnected(game_bot& bot) override = 0;

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
};

} // namespace fb::bot

#endif