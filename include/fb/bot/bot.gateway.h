#ifndef __BOT_GATEWAY_H__
#define __BOT_GATEWAY_H__

#include <fb/bot/bot.h>
#include <fb/gateway/protocol.h>
#include <shared_mutex>

namespace fb::bot {

// Forward declarations
class gateway_bot_controller;
template <typename ControllerType> class bot;

/**
 * @brief      Automated bot for testing gateway server functionality.
 *
 *             This bot specializes in connecting to gateway servers and handling
 *             initial server discovery and routing operations. It manages the
 *             gateway protocol flow including welcome messages, cryptographic
 *             setup, host discovery, and server transfers. Gateway bots are
 *             typically the first point of contact in the server architecture.
 */
class gateway_bot : public bot<gateway_bot>
{
public:
    using controller_type = gateway_bot_controller; ///< Type alias for the controller type

public:
    /**
     * @brief      Constructs a new gateway bot instance.
     *
     * @param      controller  The gateway bot controller that manages this bot.
     * @param[in]  id          The unique identifier for this bot instance.
     */
    gateway_bot(bot_controller<gateway_bot>& controller, uint32_t id);

    /**
     * @brief      Constructs a new gateway bot instance with parameters.
     *
     *             Gateway bot doesn't use transfer parameters, but this constructor
     *             is provided for compatibility with the bot creation template.
     *
     * @param      controller  The gateway bot controller that manages this bot.
     * @param[in]  id          The unique identifier for this bot instance.
     * @param[in]  params      Transfer parameters (unused for gateway bot).
     */
    gateway_bot(bot_controller<gateway_bot>& controller, uint32_t id, const fb::stream& params);

    /**
     * @brief      Destroys the gateway bot and performs cleanup.
     */
    ~gateway_bot();
};

} // namespace fb::bot

#endif