#ifndef __BOT_LOGIN_H__
#define __BOT_LOGIN_H__

#include <fb/bot/bot.h>
#include <fb/login/protocol.h>
#include <random.h>
#include <fb/encoding.h>
#include <shared_mutex>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace fb::bot {

/**
 * @brief      Automated bot for testing login server functionality.
 *
 *             This bot specializes in connecting to login servers and performing
 *             authentication-related operations. It handles login protocols,
 *             agreement acceptance, and server transfers. The bot can simulate
 *             user login behavior for testing and load simulation purposes.
 */
class login_bot : public base_bot
{
public:
    inline static std::shared_mutex _mutex;     ///< Mutex for thread-safe access to static members
    inline static uint32_t          _count = 0; ///< Global count of active login bot instances

public:
    /**
     * @brief      Constructs a new login bot instance.
     *
     * @param      owner  The bot container that manages this bot.
     * @param[in]  id     The unique identifier for this bot instance.
     */
    login_bot(bot_container& owner, uint32_t id);

    /**
     * @brief      Constructs a new login bot instance with parameters.
     *
     * @param      owner   The bot container that manages this bot.
     * @param[in]  id      The unique identifier for this bot instance.
     * @param[in]  params  Initialization parameters for the bot.
     */
    login_bot(bot_container& owner, uint32_t id, const fb::stream& params);

    /**
     * @brief      Destroys the login bot and performs cleanup.
     */
    ~login_bot();

private:
    /**
     * @brief      Generates a unique login identifier for authentication.
     *
     * @return     A randomly generated login ID string.
     */
    std::string generate_id() const;

protected:
    /**
     * @brief      Called when the bot connects to the login server.
     *
     *             Initiates the login sequence by sending authentication
     *             requests and handling the login protocol flow.
     *
     * @return     An async task that completes when connection setup is finished.
     */
    async::task<void> on_connected() override final;

    /**
     * @brief      Called when the bot disconnects from the login server.
     *
     *             Performs cleanup and updates the global bot count.
     *
     * @return     An async task that completes when disconnection cleanup is finished.
     */
    async::task<void> on_disconnected() override final;

    /**
     * @brief      Determines decryption policy for login protocol commands.
     *
     * @param[in]  cmd  The protocol command identifier.
     *
     * @return     True if the command should be decrypted, false otherwise.
     */
    bool decrypt_policy(int cmd) const;

public:
    /**
     * @brief      Handles login agreement response from the server.
     *
     *             Processes the server's agreement response and continues
     *             the login flow by accepting terms or handling errors.
     *
     * @param[in]  response  The agreement response from the login server.
     *
     * @return     An async task that completes when agreement processing is finished.
     */
    async::task<void> handle_agreement(const fb::protocol::login::response::agreement& response);

    /**
     * @brief      Handles server transfer response during login.
     *
     *             Processes transfer instructions to move the bot to
     *             a different server (typically from login to game server).
     *
     * @param[in]  response  The transfer response containing new server information.
     *
     * @return     An async task that completes when transfer processing is finished.
     */
    async::task<void> handle_transfer(const fb::protocol::response::transfer& response);
};

} // namespace fb::bot

#endif