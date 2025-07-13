#ifndef __BOT_GATEWAY_LOAD_CONTROLLER_H__
#define __BOT_GATEWAY_LOAD_CONTROLLER_H__

#include <fb/bot/gateway_controller.h>
#include <fb/gateway/protocol.h>
#include <fb/bot/gateway_bot.h>

namespace fb::bot::load {

/**
 * @brief      Gateway bot bot_controller for load testing.
 *
 *             This bot_controller implements load testing functionality for gateway bots,
 *             focusing on high-volume connection spawning and basic protocol handling.
 *             It manages bot creation, connection timing, and protocol flow optimization.
 */
class gateway_bot_controller : public fb::bot::gateway_bot_controller
{
private:
    uint32_t _remained_count; ///< Number of bots remaining to be spawned

public:
    using bot_type = gateway_bot; ///< Type alias for the managed bot type

    gateway_bot_controller(bot_container& container);

    /**
     * @brief      Initializes the load test bot_controller and sets up spawn timers.
     */
    void initialize() override;

public:
    /**
     * @brief      Handles gateway bot connection events for load testing.
     *
     *             Manages bot connections for load testing scenarios.
     *
     * @param      bot  The gateway bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(gateway_bot& bot) override;

    /**
     * @brief      Handles gateway bot disconnection events for load testing.
     *
     *             Manages bot disconnections for load testing scenarios.
     *
     * @param      bot  The gateway bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(gateway_bot& bot) override;

private:
    /**
     * @brief      Handles bot spawning for load testing.
     *
     *             Creates and connects multiple bots according to configured
     *             spawn intervals and counts for load testing purposes.
     *
     * @return     An async task that completes when bot spawning is finished.
     */
    async::task<void> handle_bot_spawn();

    /**
     * @brief      Handles welcome message from the gateway server.
     *
     *             Processes the initial welcome response and continues
     *             the gateway protocol flow.
     *
     * @param[in]  bot       The gateway bot instance.
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
     * @param[in]  bot       The gateway bot instance.
     * @param[in]  response  The encryption response containing security parameters.
     *
     * @return     An async task that completes when encryption setup is finished.
     */
    async::task<void> handle_crt(gateway_bot& bot, const fb::protocol::gateway::response::encryption& response);

    /**
     * @brief      Handles host/endpoint discovery response from the gateway.
     *
     *             Processes the list of available servers and their endpoints
     *             for subsequent connections.
     *
     * @param[in]  bot       The gateway bot instance.
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
     * @param[in]  bot       The gateway bot instance.
     * @param[in]  response  The transfer response containing new server information.
     *
     * @return     An async task that completes when transfer processing is finished.
     */
    async::task<void> handle_transfer(gateway_bot& bot, const fb::protocol::response::transfer& response);
};

} // namespace fb::bot::load

#endif