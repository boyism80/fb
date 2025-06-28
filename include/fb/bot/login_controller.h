#ifndef __BOT_LOGIN_CONTROLLER_H__
#define __BOT_LOGIN_CONTROLLER_H__

#include <fb/bot/controller.h>
#include <fb/login/protocol.h>
#include <fb/bot/login_bot.h>

namespace fb::bot {

/**
 * @brief      Abstract base class for login bot bot_controllers.
 *
 *             This class defines the common interface for all login bot bot_controllers.
 *             It provides the foundation for both load testing and integration testing
 *             bot_controllers while maintaining a consistent interface for bot management.
 */
class login_bot_controller : public bot_controller<login_bot>
{
public:
    using bot_type = login_bot; ///< Type alias for the managed bot type

protected:
    /**
     * @brief      Protected constructor to enforce abstract nature of this class.
     *
     *             Only derived classes can instantiate this bot_controller.
     *
     * @param      container  The bot container that manages this bot_controller.
     */
    login_bot_controller(bot_container& container);

public:
    /**
     * @brief      Virtual destructor for proper cleanup in derived classes.
     */
    virtual ~login_bot_controller() = default;

    /**
     * @brief      Initializes the login bot_controller and sets up timers.
     *
     *             This method must be implemented by derived classes to provide
     *             specific initialization logic for different test modes.
     */
    virtual void initialize() = 0;

    /**
     * @brief      Login-specific decryption policy.
     *
     *             Login bots do not decrypt certain protocol commands
     *             like transfer messages.
     *
     * @param[in]  cmd  The protocol command identifier.
     *
     * @return     False for transfer commands, true for others.
     */
    bool decrypt_policy(int cmd) const override final;

public:
    /**
     * @brief      Handles login bot connection events.
     *
     *             This method is called when a bot successfully connects to the login server.
     *             Derived classes should implement specific connection handling logic.
     *
     * @param      bot  The login bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(login_bot& bot) override = 0;

    /**
     * @brief      Handles login bot disconnection events.
     *
     *             This method is called when a bot disconnects from the login server.
     *             Derived classes should implement specific disconnection handling logic.
     *
     * @param      bot  The login bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(login_bot& bot) override = 0;
};

} // namespace fb::bot

#endif