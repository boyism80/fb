#ifndef __BOT_LOGIN_INTEGRATION_CONTROLLER_H__
#define __BOT_LOGIN_INTEGRATION_CONTROLLER_H__

#include <fb/bot/login_controller.h>
#include <fb/login/protocol.h>
#include <fb/bot/login_bot.h>

namespace fb::bot::integration {

/**
 * @brief      Login bot bot_controller for integration testing.
 *
 *             This bot_controller implements sophisticated integration testing
 *             capabilities for login bots, focusing on authentication flow validation,
 *             account management testing, and comprehensive login behavior verification.
 */
class login_bot_controller : public fb::bot::login_bot_controller
{
public:
    using bot_type = login_bot; ///< Type alias for the managed bot type

    login_bot_controller(bot_container& container);

    /**
     * @brief      Initializes the integration test bot_controller.
     *
     *             Sets up test scenarios, validation handlers, and
     *             prepares the test execution environment for login testing.
     */
    void initialize() override;

public:
    /**
     * @brief      Handles login bot connection events for integration testing.
     *
     *             Initiates login-specific test scenarios and sets up
     *             validation mechanisms when a bot connects.
     *
     * @param      bot  The login bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(login_bot& bot) override;

    /**
     * @brief      Handles login bot disconnection events for integration testing.
     *
     *             Collects test results and performs cleanup when a login bot disconnects.
     *
     * @param      bot  The login bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(login_bot& bot) override;

private:
    /**
     * @brief      Handles login agreement with integration test validation.
     *
     *             Tests authentication protocol compliance and validates
     *             account creation/login flow correctness.
     *
     * @param[in]  bot       The login bot instance.
     * @param[in]  response  The agreement response from the login server.
     *
     * @return     An async task that completes when agreement processing is finished.
     */
    async::task<void> handle_agreement(login_bot& bot, const fb::protocol::login::response::agreement& response);

    /**
     * @brief      Handles server transfer with integration test validation.
     *
     *             Tests login-to-game server transition mechanics and validates
     *             seamless authentication token transfer.
     *
     * @param[in]  bot       The login bot instance.
     * @param[in]  response  The transfer response containing new server information.
     *
     * @return     An async task that completes when transfer processing is finished.
     */
    async::task<void> handle_transfer(login_bot& bot, const fb::protocol::response::transfer& response);
};

} // namespace fb::bot::integration

#endif