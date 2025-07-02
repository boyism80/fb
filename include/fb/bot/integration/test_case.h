#ifndef __BOT_INTEGRATION_TEST_CASE_H__
#define __BOT_INTEGRATION_TEST_CASE_H__

#include <fb/bot/game_bot.h>
#include <fb/locker.h>
#include <async/task.h>
#include <memory>
#include <vector>
#include <atomic>
#include <string>
#include <boost/asio.hpp>

namespace fb::bot::integration {

// Forward declarations
class game_bot_controller;

/**
 * @brief      Abstract base class for bot integration test cases.
 *
 *             Provides a framework for creating various types of integration tests
 *             such as movement, attack, skill tests, etc. Each test manages its own
 *             state, spawns bots as needed, and cleans up resources when complete.
 */
class bot_integration_test
{
public:
    virtual ~bot_integration_test() = default;

    /**
     * @brief      Initializes the test case and spawns required bots if needed.
     *
     *             Each test implementation can override this method to perform
     *             test-specific initialization and bot spawning. The default
     *             implementation does nothing, allowing tests that don't need
     *             bots to skip the spawning process.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     * @param[in]  endpoint    The server endpoint to connect bots to.
     *
     * @return     A task that completes when initialization is finished.
     */
    virtual async::task<void> initialize(game_bot_controller& controller)
    {
        // Default implementation does nothing - tests can override as needed
        co_return;
    }

    /**
     * @brief      Executes the integration test with the spawned bots.
     *
     *             This method is called after all bots are ready and starts
     *             the actual test execution logic.
     *
     * @return     A task that completes when the test finishes, returning true on success.
     */
    virtual async::task<bool> execute() = 0;

    /**
     * @brief      Checks if the test has completed.
     *
     * @return     True if the test is complete, false otherwise.
     */
    virtual bool is_complete() const = 0;

    /**
     * @brief      Resets the test state to initial conditions.
     */
    virtual void reset() = 0;

    /**
     * @brief      Gets the name of the test for logging and identification.
     *
     * @return     The test name as a string.
     */
    virtual std::string name() const = 0;

    /**
     * @brief      Checks if the test is currently in progress.
     *
     * @return     True if the test is running, false otherwise.
     */
    virtual bool is_running() const = 0;

    /**
     * @brief      Checks if all spawned bots are ready to start the test.
     *
     *             Each test implementation defines its own readiness criteria.
     *             For example, movement test might require all bots to have non-zero oid,
     *             while attack test might require bots to be in specific positions.
     *
     * @return     True if all spawned bots are ready to start this test, false otherwise.
     */
    virtual bool is_ready() const = 0;

    /**
     * @brief      Called when a bot connects to the test.
     *
     *             Each test implementation can decide whether to store this bot
     *             or ignore it based on its own requirements.
     *
     * @param[in]  bot  Shared pointer to the connected bot.
     */
    virtual void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) = 0;

    /**
     * @brief      Cleans up test resources and disconnects all spawned bots.
     *
     *             This method is called when the test completes or needs to be stopped.
     *             It ensures all bot connections are properly closed and resources are freed.
     */
    virtual void cleanup();
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_TEST_CASE_H__