#ifndef __BOT_GATEWAY_H__
#define __BOT_GATEWAY_H__

#include <fb/bot/bot.h>
#include <fb/gateway/protocol.h>
#include <shared_mutex>

namespace fb::bot {

/**
 * @brief      Automated bot for testing gateway server functionality.
 *
 *             This bot specializes in connecting to gateway servers and handling
 *             initial server discovery and routing operations. It manages the
 *             gateway protocol flow including welcome messages, cryptographic
 *             setup, host discovery, and server transfers. Gateway bots are
 *             typically the first point of contact in the server architecture.
 */
class gateway_bot : public base_bot
{
public:
    inline static std::shared_mutex _mutex;     ///< Mutex for thread-safe access to static members
    inline static uint32_t          _count = 0; ///< Global count of active gateway bot instances

public:
    /**
     * @brief      Constructs a new gateway bot instance.
     *
     * @param      owner  The bot container that manages this bot.
     * @param[in]  id     The unique identifier for this bot instance.
     */
    gateway_bot(bot_container& owner, uint32_t id);

    /**
     * @brief      Destroys the gateway bot and performs cleanup.
     */
    ~gateway_bot();

protected:
    /**
     * @brief      Called when the bot connects to the gateway server.
     *
     *             Initiates the gateway protocol sequence and begins
     *             server discovery and authentication processes.
     *
     * @return     An async task that completes when connection setup is finished.
     */
    async::task<void> on_connected() override final;

    /**
     * @brief      Called when the bot disconnects from the gateway server.
     *
     *             Performs cleanup and updates the global bot count.
     *
     * @return     An async task that completes when disconnection cleanup is finished.
     */
    async::task<void> on_disconnected() override final;

private:
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
    async::task<void> handle_welcome(const fb::protocol::gateway::response::welcome& response);

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
    async::task<void> handle_crt(const fb::protocol::gateway::response::crypto& response);

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
    async::task<void> handle_hosts(const fb::protocol::gateway::response::endpoint& response);

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
    async::task<void> handle_transfer(const fb::protocol::response::transfer& response);

protected:
    /**
     * @brief      Determines decryption policy for gateway protocol commands.
     *
     * @param[in]  cmd  The protocol command identifier.
     *
     * @return     True if the command should be decrypted, false otherwise.
     */
    bool decrypt_policy(int cmd) const;
};

} // namespace fb::bot

#endif