#ifndef __BOT_GAME_INTEGRATION_CONTROLLER_H__
#define __BOT_GAME_INTEGRATION_CONTROLLER_H__

#include <fb/bot/game_controller.h>
#include <fb/game/protocol.h>
#include <fb/bot/game_bot.h>

namespace fb::bot::integration {

/**
 * @brief      Game bot bot_controller for integration testing.
 *
 *             This bot_controller implements sophisticated integration testing
 *             capabilities including scenario-based testing, response verification,
 *             and comprehensive test case management. Unlike load testing,
 *             it focuses on functional correctness and system behavior validation.
 */
class game_bot_controller : public fb::bot::game_bot_controller
{
public:
    using bot_type = game_bot; ///< Type alias for the managed bot type

    game_bot_controller(bot_container& container);

    /**
     * @brief      Initializes the integration test bot_controller.
     *
     *             Sets up test scenarios, response validation handlers,
     *             and prepares the test execution environment.
     */
    void initialize() override;

public:
    /**
     * @brief      Handles game bot connection events for integration testing.
     *
     *             Initiates test scenarios and sets up validation mechanisms
     *             when a bot connects to the game server.
     *
     * @param      bot  The game bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(game_bot& bot) override;

    /**
     * @brief      Handles game bot disconnection events for integration testing.
     *
     *             Collects test results and performs cleanup when a bot disconnects.
     *
     * @param      bot  The game bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(game_bot& bot) override;

private:
    /**
     * @brief      Handles timer events for integration test scenarios.
     *
     *             Executes scheduled test cases and monitors test progress.
     *
     * @param[in]  now  The current date and time.
     * @param[in]  id   The thread identifier.
     *
     * @return     An async task that completes when timer processing is finished.
     */
    async::task<void> handle_timer(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      Handles game time updates with integration test logic.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The time response containing current game time.
     *
     * @return     An async task that completes when time processing is finished.
     */
    async::task<void> handle_time(game_bot& bot, const fb::protocol::game::response::time& response);

    /**
     * @brief      Handles character state updates with validation.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The internal state update response.
     *
     * @return     An async task that completes when state processing is finished.
     */
    async::task<void> handle_state(game_bot& bot, const fb::protocol::game::response::update_internal& response);

    /**
     * @brief      Handles map configuration with integration test validation.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The map config response containing map data.
     *
     * @return     An async task that completes when map processing is finished.
     */
    async::task<void> handle_map(game_bot& bot, const fb::protocol::game::response::map_config& response);

    /**
     * @brief      Handles message notifications with test case execution.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The message response containing notification text.
     *
     * @return     An async task that completes when message processing is finished.
     */
    async::task<void> handle_message(game_bot& bot, const fb::protocol::game::response::message& response);

    /**
     * @brief      Handles sequence ID updates with validation.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The ID response containing the new sequence number.
     *
     * @return     An async task that completes when sequence processing is finished.
     */
    async::task<void> handle_sequence(game_bot& bot, const fb::protocol::game::response::id& response);

    /**
     * @brief      Handles position updates with movement validation.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The position response containing new coordinates.
     *
     * @return     An async task that completes when position processing is finished.
     */
    async::task<void> handle_position(game_bot& bot, const fb::protocol::game::response::position& response);

    /**
     * @brief      Handles movement notifications with path validation.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The move response containing movement data.
     *
     * @return     An async task that completes when movement processing is finished.
     */
    async::task<void> handle_move(game_bot& bot, const fb::protocol::game::response::move& response);

    /**
     * @brief      Handles server transfer with connection testing.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The transfer response containing new server information.
     *
     * @return     An async task that completes when transfer processing is finished.
     */
    async::task<void> handle_transfer(game_bot& bot, const fb::protocol::response::transfer& response);
};

} // namespace fb::bot::integration

#endif