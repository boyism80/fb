#ifndef __BOT_LOGIN_LOAD_CONTROLLER_H__
#define __BOT_LOGIN_LOAD_CONTROLLER_H__

#include <fb/bot/login_controller.h>
#include <fb/login/protocol.h>
#include <fb/bot/login_bot.h>

namespace fb::bot::load {

/**
 * @brief      Login bot bot_controller for load testing.
 *
 *             This bot_controller implements load testing functionality for login bots,
 *             focusing on automated account creation, login flow testing,
 *             and high-volume user session management.
 */
class login_bot_controller : public fb::bot::login_bot_controller
{
public:
    using bot_type = login_bot; ///< Type alias for the managed bot type

    login_bot_controller(bot_container& container);

    /**
     * @brief      Initializes the load test bot_controller.
     *
     *             Sets up protocol handlers for load testing scenarios.
     */
    void initialize() override;

public:
    /**
     * @brief      Handles login bot connection events for load testing.
     *
     *             Manages bot connections and initiates login flow for load testing.
     *
     * @param      bot  The login bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(login_bot& bot) override;

    /**
     * @brief      Handles login bot disconnection events for load testing.
     *
     *             Manages bot disconnections for load testing scenarios.
     *
     * @param      bot  The login bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(login_bot& bot) override;

private:
    /**
     * @brief      Handles login agreement response from the server.
     *
     *             Processes the server's agreement response and continues
     *             the login flow by accepting terms or handling errors.
     *
     * @param[in]  bot       The login bot instance.
     * @param[in]  response  The agreement response from the login server.
     *
     * @return     An async task that completes when agreement processing is finished.
     */
    async::task<void> handle_agreement(login_bot& bot, const fb::protocol::login::response::agreement& response);

    /**
     * @brief      Handles server transfer response during login.
     *
     *             Processes transfer instructions to move the bot to
     *             a different server (typically from login to game server).
     *
     * @param[in]  bot       The login bot instance.
     * @param[in]  response  The transfer response containing new server information.
     *
     * @return     An async task that completes when transfer processing is finished.
     */
    async::task<void> handle_transfer(login_bot& bot, const fb::protocol::response::transfer& response);
};

} // namespace fb::bot::load

#endif