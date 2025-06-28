#ifndef __BOT_GATEWAY_CONTROLLER_H__
#define __BOT_GATEWAY_CONTROLLER_H__

#include <fb/bot/bot.controller.h>
#include <fb/gateway/protocol.h>
#include <fb/bot/bot.gateway.h>

namespace fb::bot {

class gateway_bot_controller : public bot_controller<gateway_bot>
{
private:
    uint32_t _remained_count; ///< Number of bots remaining to be spawned

public:
    using bot_type = gateway_bot; ///< Type alias for the managed bot type

    gateway_bot_controller(bot_container& container);

    /**
     * @brief      Initializes the gateway controller and sets up timers.
     */
    void initialize();

protected:
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
    virtual bool decrypt_policy(int cmd) const override;

public:
    /**
     * @brief      Handles gateway bot connection events.
     *
     *             Increments the gateway bot counter when a bot connects.
     *
     * @param      bot  The gateway bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(gateway_bot& bot) override;

    /**
     * @brief      Handles gateway bot disconnection events.
     *
     *             Decrements the gateway bot counter when a bot disconnects.
     *
     * @param      bot  The gateway bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(gateway_bot& bot) override;

private:
    async::task<void> handle_bot_spawn();

    /**
     * @brief      Handles welcome message from the gateway server.
     *
     *             Processes the initial welcome response and continues
     *             the gateway protocol flow.
     *
     * @param[in]  response  The welcome response from the gateway server.
     *
     * @return     An async task that completes when welcome processing is finished.
     */
    async::task<void> handle_welcome(gateway_bot& bot, const fb::protocol::gateway::response::welcome& response);

    /**
     * @brief      Handles cryptographic setup response from the gateway.
     *
     *             Processes cryptographic parameters and establishes
     *             secure communication with the gateway server.
     *
     * @param[in]  response  The crypto response containing security parameters.
     *
     * @return     An async task that completes when crypto setup is finished.
     */
    async::task<void> handle_crt(gateway_bot& bot, const fb::protocol::gateway::response::crypto& response);

    /**
     * @brief      Handles host/endpoint discovery response from the gateway.
     *
     *             Processes the list of available servers and their endpoints
     *             for subsequent connections.
     *
     * @param[in]  response  The endpoint response containing server information.
     *
     * @return     An async task that completes when host processing is finished.
     */
    async::task<void> handle_hosts(gateway_bot& bot, const fb::protocol::gateway::response::endpoint& response);

    /**
     * @brief      Handles server transfer response from the gateway.
     *
     *             Processes transfer instructions to move the bot to
     *             a different server (typically to login or game servers).
     *
     * @param[in]  response  The transfer response containing new server information.
     *
     * @return     An async task that completes when transfer processing is finished.
     */
    async::task<void> handle_transfer(gateway_bot& bot, const fb::protocol::response::transfer& response);
};

} // namespace fb::bot

#endif