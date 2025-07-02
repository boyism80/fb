#ifndef __BOT_GAME_INTEGRATION_CONTROLLER_H__
#define __BOT_GAME_INTEGRATION_CONTROLLER_H__

#include <fb/bot/game_controller.h>
#include <fb/game/protocol.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/integration/test_case.h>
#include <fb/locker.h>
#include <memory>
#include <queue>

namespace fb::bot::integration {

/**
 * @brief      Game bot bot_controller for integration testing.
 *
 *             This bot_controller implements sophisticated integration testing
 *             capabilities including scenario-based testing, response verification,
 *             and comprehensive test case management. Unlike load testing,
 *             it focuses on functional correctness and system behavior validation.
 *
 *             The controller manages various test cases (movement, attack, skill tests)
 *             and coordinates their execution across multiple bots.
 */
class game_bot_controller : public fb::bot::game_bot_controller
{
private:
    fb::locker<std::unique_ptr<bot_integration_test>>
        _current_test; ///< Currently active test case with thread-safe access
    std::queue<std::unique_ptr<bot_integration_test>> _test_queue; ///< Queue of tests to execute in oid

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

public:
    /**
     * @brief      Sets the current integration test case.
     *
     *             Replaces the current test with a new one and resets its state.
     *             If a test is currently running, it will be stopped first.
     *
     * @param      test  Unique pointer to the new test case.
     */
    void set_test(std::unique_ptr<bot_integration_test> test);

    /**
     * @brief      Starts the current integration test.
     *
     *             Executes the currently set test case with all connected bots.
     *             If no test is set or a test is already running, this method returns immediately.
     *
     * @return     An async task that completes when the test finishes, returning true on success.
     */
    async::task<bool> start_test();

    /**
     * @brief      Resets the current test case to initial state.
     *
     *             Stops any running test and resets its internal state.
     */
    void reset_current_test();

private:
    /**
     * @brief      Starts the next test from the queue.
     *
     *             Moves to the next test in the queue and initializes it.
     *             If the queue is empty, logs completion of all tests.
     */
    void start_next_test();

private:
    /**
     * @brief      Handles timer events for integration test scenarios.
     *
     *             Executes scheduled test cases and monitors test progress.
     *             Checks if all bots are ready according to current test criteria
     *             and starts the test automatically when conditions are met.
     *
     * @param[in]  now  The current date and time.
     *
     * @return     An async task that completes when timer processing is finished.
     */
    async::task<void> handle_timer();

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
     * @brief      Handles oid ID updates with test case triggering.
     *
     *             When a bot receives a oid ID, it signals readiness to the current test.
     *             For movement tests, this triggers the signal_bot_ready() method.
     *
     * @param[in]  bot       The game bot instance.
     * @param[in]  response  The ID response containing the new oid number.
     *
     * @return     An async task that completes when oid processing is finished.
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