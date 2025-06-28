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

// Forward declarations
class login_bot_controller;
template <typename ControllerType> class bot;

/**
 * @brief      Automated bot for testing login server functionality.
 *
 *             This bot specializes in connecting to login servers and performing
 *             authentication-related operations. It handles login protocols,
 *             agreement acceptance, and server transfers. The bot can simulate
 *             user login behavior for testing and load simulation purposes.
 */
class login_bot : public bot<login_bot>
{
public:
    using bot_controller_type = login_bot_controller; ///< Type alias for the bot_controller type

public:
    /**
     * @brief      Constructs a new login bot instance.
     *
     * @param      bot_controller  The login bot bot_controller that manages this bot.
     * @param[in]  id          The unique identifier for this bot instance.
     */
    login_bot(bot_controller<login_bot>& bot_controller, uint32_t id);

    /**
     * @brief      Constructs a new login bot instance with transfer parameters.
     *
     * @param      bot_controller  The login bot bot_controller that manages this bot.
     * @param[in]  id          The unique identifier for this bot instance.
     * @param[in]  params      Transfer parameters containing crypto information.
     */
    login_bot(bot_controller<login_bot>& bot_controller, uint32_t id, const fb::stream& params);

    /**
     * @brief      Destroys the login bot and performs cleanup.
     */
    ~login_bot();

public:
    /**
     * @brief      Generates a unique login identifier for authentication.
     *
     * @return     A randomly generated login ID string.
     */
    std::string generate_id() const;
};

} // namespace fb::bot

#endif