#ifndef __BOT_GATEWAY_CONTROLLER_H__
#define __BOT_GATEWAY_CONTROLLER_H__

#include <fb/bot/controller.h>
#include <fb/gateway/protocol.h>
#include <fb/bot/gateway_bot.h>

namespace fb::bot {

/**
 * @brief      Abstract base class for gateway bot bot_controllers.
 *
 *             This class defines the common interface for all gateway bot bot_controllers.
 *             It provides the foundation for both load testing and integration testing
 *             bot_controllers while maintaining a consistent interface for bot management.
 */
class gateway_bot_controller : public bot_controller<gateway_bot>
{
public:
    using bot_type = gateway_bot; ///< Type alias for the managed bot type

protected:
    /**
     * @brief      Protected constructor to enforce abstract nature of this class.
     *
     *             Only derived classes can instantiate this bot_controller.
     *
     * @param      container  The bot container that manages this bot_controller.
     */
    gateway_bot_controller(bot_container& container);

public:
    /**
     * @brief      Virtual destructor for proper cleanup in derived classes.
     */
    virtual ~gateway_bot_controller() = default;

    /**
     * @brief      Initializes the gateway bot_controller and sets up timers.
     *
     *             This method must be implemented by derived classes to provide
     *             specific initialization logic for different test modes.
     */
    virtual void initialize() = 0;

    /**
     * @brief      Gateway-specific decryption policy.
     *
     *             Gateway bots do not decrypt certain protocol commands
     *             like welcome (0x00) and crypto exchange (0x7E).
     *
     * @param[in]  cmd  The protocol command identifier.
     *
     * @return     False for commands 0x00 and 0x7E, true for others.
     */
    bool decrypt_policy(int cmd) const override final;

public:
    /**
     * @brief      Handles gateway bot connection events.
     *
     *             This method is called when a bot successfully connects to the gateway server.
     *             Derived classes should implement specific connection handling logic.
     *
     * @param      bot  The gateway bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(gateway_bot& bot) override = 0;

    /**
     * @brief      Handles gateway bot disconnection events.
     *
     *             This method is called when a bot disconnects from the gateway server.
     *             Derived classes should implement specific disconnection handling logic.
     *
     * @param      bot  The gateway bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(gateway_bot& bot) override = 0;
};

} // namespace fb::bot

#endif