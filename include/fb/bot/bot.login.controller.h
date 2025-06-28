#ifndef __BOT_LOGIN_CONTROLLER_H__
#define __BOT_LOGIN_CONTROLLER_H__

#include <fb/bot/bot.controller.h>
#include <fb/login/protocol.h>
#include <fb/bot/bot.login.h>

namespace fb::bot {

class login_bot_controller : public bot_controller<login_bot>
{
public:
    using bot_type = login_bot; ///< Type alias for the managed bot type

    login_bot_controller(bot_container& container);

public:
    /**
     * @brief      Handles login bot connection events.
     *
     *             Increments the login bot counter when a bot connects.
     *
     * @param      bot  The login bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(login_bot& bot) override;

    /**
     * @brief      Handles login bot disconnection events.
     *
     *             Decrements the login bot counter when a bot disconnects.
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
    virtual bool decrypt_policy(int cmd) const override;
};

} // namespace fb::bot

#endif