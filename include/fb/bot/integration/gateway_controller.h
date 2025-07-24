#ifndef __BOT_GATEWAY_INTEGRATION_CONTROLLER_H__
#define __BOT_GATEWAY_INTEGRATION_CONTROLLER_H__

#include <fb/bot/gateway_controller.h>
#include <fb/gateway/protocol.h>
#include <fb/bot/gateway_bot.h>

namespace fb::bot::integration {

/**
 * @brief      Gateway bot bot_controller for integration testing.
 *
 *             This bot_controller implements sophisticated integration testing
 *             capabilities for gateway bots, focusing on protocol validation,
 *             connection quality testing, and comprehensive behavior verification.
 */
class gateway_bot_controller : public fb::bot::gateway_bot_controller
{
private:
    using super = fb::bot::gateway_bot_controller;

public:
    using bot_type = gateway_bot; ///< Type alias for the managed bot type

    gateway_bot_controller(bot_container& container);

    /**
     * @brief      Initializes the integration test bot_controller.
     *
     *             Sets up test scenarios, validation handlers, and
     *             prepares the test execution environment for gateway testing.
     */
    void initialize() override;

public:
    /**
     * @brief      Handles gateway bot connection events for integration testing.
     *
     *             Initiates gateway-specific test scenarios and sets up
     *             validation mechanisms when a bot connects.
     *
     * @param      bot  The gateway bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(gateway_bot& bot) override;

    /**
     * @brief      Handles gateway bot disconnection events for integration testing.
     *
     *             Collects test results and performs cleanup when a gateway bot disconnects.
     *
     * @param      bot  The gateway bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(gateway_bot& bot) override;

private:
    /**
     * @brief      Handles welcome message with integration test validation.
     *
     *             Validates welcome protocol compliance and tests response timing.
     *
     * @param[in]  bot       The gateway bot instance.
     * @param[in]  response  The welcome response from the gateway server.
     *
     * @return     An async task that completes when welcome processing is finished.
     */
    async::task<void> handle_welcome(gateway_bot& bot, const fb::protocol::gateway::response::welcome& response);

    /**
     * @brief      Handles cryptographic setup with security validation.
     *
     *             Tests cryptographic protocol implementation and validates
     *             security parameter correctness.
     *
     * @param[in]  bot       The gateway bot instance.
     * @param[in]  response  The encryption response containing security parameters.
     *
     * @return     An async task that completes when encryption setup is finished.
     */
    async::task<void> handle_crt(gateway_bot& bot, const fb::protocol::gateway::response::encryption& response);

    /**
     * @brief      Handles host discovery with endpoint validation.
     *
     *             Validates server endpoint information and tests
     *             service discovery functionality.
     *
     * @param[in]  bot       The gateway bot instance.
     * @param[in]  response  The endpoint response containing server information.
     *
     * @return     An async task that completes when host processing is finished.
     */
    async::task<void> handle_hosts(gateway_bot& bot, const fb::protocol::gateway::response::endpoint& response);

    /**
     * @brief      Handles server transfer with transition validation.
     *
     *             Tests server transfer mechanics and validates
     *             seamless connection transitions.
     *
     * @param[in]  bot       The gateway bot instance.
     * @param[in]  response  The transfer response containing new server information.
     *
     * @return     An async task that completes when transfer processing is finished.
     */
    async::task<void> handle_transfer(gateway_bot& bot, const fb::protocol::response::transfer& response);
};

} // namespace fb::bot::integration

#endif