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
#include <chrono>
#include <functional>
#include <optional>

namespace fb::bot::integration {

// Forward declarations
class game_bot_controller;

/**
 * @brief      Structure for spawned monster information
 */
struct spawned_monster_info
{
    uint32_t                   oid;      ///< Monster object ID
    fb::model::point<uint16_t> position; ///< Monster position
    uint32_t                   look;     ///< Monster look value
};

/**
 * @brief      Abstract base class for bot integration test cases.
 *
 *             Provides a framework for creating various types of integration tests
 *             such as movement, attack, skill tests, etc. Each test manages its own
 *             state, spawns bots as needed, and cleans up resources when complete.
 *             Tests can be chained together to form a linked list for sequential execution.
 */
class bot_integration_test
{
public:
    enum class test_state : uint8_t
    {
        idle,      ///< Test is not started yet
        ready,     ///< Test is ready to start
        running,   ///< Test is currently executing
        completed, ///< Test has completed successfully
        failed     ///< Test has failed
    };

private:
    std::vector<std::shared_ptr<fb::bot::game_bot>> _test_bots; ///< Collection of bots managed by this test
    test_state                                      _state{test_state::idle}; ///< Current state of the test
    game_bot_controller&                            _controller; ///< Reference to the parent game bot controller

public:
    const uint32_t bot_count; ///< Number of bots required for this test

protected:
#ifdef _DEBUG
    static constexpr auto DEFAULT_TIMEOUT  = 1h;
    static constexpr auto DEFAULT_INTERVAL = 100ms;
#else
    static constexpr auto DEFAULT_TIMEOUT  = 10s;
    static constexpr auto DEFAULT_INTERVAL = 100ms;
#endif

public:
    /**
     * @brief      Constructs a new bot integration test with controller reference.
     *
     * @param[in]  controller  Reference to the game bot controller.
     * @param[in]  bot_count   Number of bots to spawn.
     */
    bot_integration_test(game_bot_controller& controller, uint32_t bot_count = 1);

    virtual ~bot_integration_test() = default;

    /**
     * @brief      Gets the current state of the test.
     *
     * @return     The current test state.
     */
    test_state get_state() const;

    /**
     * @brief      Sets the current state of the test.
     *
     * @param[in]  state  The new test state.
     */
    void set_state(test_state state);

    /**
     * @brief      Checks if the test has completed execution.
     *
     * @return     True if the test is complete, false otherwise.
     */
    bool is_complete() const;

    /**
     * @brief      Checks if the test is currently running.
     *
     * @return     True if the test is running, false otherwise.
     */
    bool is_running() const;

    /**
     * @brief      Called when a bot connects to the test.
     *
     *             Default implementation adds the bot to the test's bot collection
     *             and logs the connection with the test name.
     *
     * @param[in]  bot  Shared pointer to the connected bot.
     */
    virtual void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot);

    /**
     * @brief      Called when a bot disconnects from the test.
     *
     *             Default implementation logs the disconnection with the test name.
     *
     * @param[in]  bot  Shared pointer to the disconnected bot.
     */
    virtual void on_bot_disconnected(std::shared_ptr<fb::bot::game_bot> bot);

    /**
     * @brief      Notifies the controller that this test is ready to start.
     *
     *             Called by derived classes when they detect they are ready to begin execution.
     */
    void notify_ready();

    /**
     * @brief      Notifies the controller that this test has completed.
     *
     *             Called by derived classes when they detect they have completed execution.
     */
    void notify_completed();

    /**
     * @brief      Performs cleanup operations when the test is destroyed.
     *
     *             Default implementation disconnects all bots and logs cleanup completion.
     */
    virtual void cleanup();

    /**
     * @brief      Checks if the test is ready to start execution.
     *
     *             Default implementation checks if we have the required number of bots
     *             and that all bots have valid OID and position.
     *
     * @return     True if the test is ready to start, false otherwise.
     */
    virtual bool is_ready() const;

    /**
     * @brief      Gets the current list of test bots.
     *
     * @return     Vector of shared pointers to the test bots.
     */
    std::vector<std::shared_ptr<fb::bot::game_bot>> get_test_bots() const;

    /**
     * @brief      Initializes the test case and spawns required bots.
     *
     *             Default implementation spawns the required number of gateway bots
     *             and connects them to the game server.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     *
     * @return     A task that completes when initialization is finished.
     */
    virtual async::task<void> initialize(game_bot_controller& controller);

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
     * @brief      Gets the name of the test for logging and identification.
     *
     * @return     The test name as a string.
     */
    virtual std::string name() const = 0;

protected:
    /**
     * @brief      Resets the test state to idle.
     *
     *             This method should be overridden by derived classes to
     *             implement specific reset logic.
     */
    virtual void reset();

    /**
     * @brief      Common hook handler for sequence (object ID) responses.
     *
     *             Checks if all bots are ready and notifies the controller when ready.
     *
     * @param[in]  bot       The bot that received the response.
     * @param[in]  response  The sequence response containing object ID.
     *
     * @return     A task that completes when the hook processing is done.
     */
    virtual async::task<void> on_hook_sequence(fb::bot::game_bot&                      bot,
                                               const fb::protocol::game::response::id& response);

    /**
     * @brief      Common hook handler for position responses.
     *
     *             Checks if all bots are ready and notifies the controller when ready.
     *
     * @param[in]  bot       The bot that received the response.
     * @param[in]  response  The position response.
     *
     * @return     A task that completes when the hook processing is done.
     */
    virtual async::task<void> on_hook_position(fb::bot::game_bot&                            bot,
                                               const fb::protocol::game::response::position& response);
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_TEST_CASE_H__