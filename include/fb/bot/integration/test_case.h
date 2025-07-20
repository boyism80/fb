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
#include <fb/generator.h>

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
    using test_bots_t = std::vector<std::shared_ptr<fb::bot::game_bot>>;
    using scenario_t  = std::function<async::task<bool>()>;

    enum class test_state : uint8_t
    {
        idle,      ///< Test is not started yet
        ready,     ///< Test is ready to start
        running,   ///< Test is currently executing
        completed, ///< Test has completed successfully
        failed     ///< Test has failed
    };

    struct parallel_scenarios_context
    {
        std::unordered_map<uint32_t, std::queue<scenario_t>> queues;
        std::shared_ptr<async::task_completion_source<bool>> promise;
        std::atomic<int>                                     processed;
        std::atomic<bool>                                    success;
        int                                                  count;
    };

private:
    test_bots_t            _test_bots;               ///< Collection of bots managed by this test
    test_state             _state{test_state::idle}; ///< Current state of the test
    std::queue<scenario_t> _scenario_queue;          ///< Queue of scenario functions to execute

public:
    game_bot_controller& controller; ///< Reference to the parent game bot controller
    const uint32_t       bot_count;  ///< Number of bots required for this test

protected:
#ifdef _DEBUG
    static constexpr auto DEFAULT_TIMEOUT  = 1h;
    static constexpr auto DEFAULT_INTERVAL = 100ms;
#else
    static constexpr auto DEFAULT_TIMEOUT  = 10s;
    static constexpr auto DEFAULT_INTERVAL = 100ms;
#endif

protected:
    /**
     * @brief      Constructs a new bot integration test with controller reference.
     *
     * @param[in]  controller  Reference to the game bot controller.
     * @param[in]  bot_count   Number of bots to spawn.
     */
    bot_integration_test(game_bot_controller& controller, uint32_t bot_count = 1);

public:
    virtual ~bot_integration_test() = default;

private:
    /**
     * @brief      Executes a single parallel scenario within the context.
     *
     *             This method processes scenarios from the specified queue index
     *             within the parallel scenarios context. It handles scenario execution,
     *             progress tracking, and result aggregation in a thread-safe manner.
     *
     * @param[in]  context  Shared pointer to the parallel scenarios context.
     * @param[in]  index    The queue index to process scenarios from.
     *
     * @return     A task that completes when the scenario execution is finished.
     *
     * @note       This method is called internally by parallel_scenarios()
     * @note       Each call processes scenarios from a specific queue index
     */
    async::task<void> execute_parallel_scenario(std::shared_ptr<parallel_scenarios_context> context, uint32_t index);

protected:
    /**
     * @brief      Generates a scenario function for the test.
     *
     * @return     A task that completes when the scenario is generated.
     */
    virtual generator<scenario_t> on_generate_scenario() = 0;

public:
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
     * @brief      Called when the test is activated.
     *
     *             This method is called when the test is activated by the controller.
     *             It is used to perform any initialization that needs to be done when the test is activated.
     *
     * @param[in]  controller  The game bot controller.
     *
     * @return     A task that completes when the test is activated.
     */
    virtual async::task<void> on_active(game_bot_controller& controller);

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
    virtual async::task<void> on_initialize(game_bot_controller& controller);

    /**
     * @brief      Performs cleanup operations when the test is destroyed.
     *
     *             Default implementation disconnects all bots and logs cleanup completion.
     */
    virtual async::task<void> on_finished();

    /**
     * @brief      Executes the integration test with the spawned bots.
     *
     *             This method is called after all bots are ready and starts
     *             the actual test execution logic.
     *
     * @return     A task that completes when the test finishes, returning true on success.
     */
    async::task<bool> execute();

    /**
     * @brief      Gets the name of the test for logging and identification.
     *
     * @return     The test name as a string.
     */
    virtual std::string name() const = 0;

protected:
    /**
     * @brief      Called when a scenario is started.
     *
     * @param[in]  scenario_index  The index of the scenario.
     */
    virtual async::task<void> on_scenario_started(uint32_t scenario_index);

    /**
     * @brief      Called when a scenario is finished.
     *
     * @param[in]  scenario_index  The index of the scenario.
     */
    virtual async::task<void> on_scenario_finished(uint32_t scenario_index);

    /**
     * @brief      Called when a parallel scenario is started.
     *
     * @param[in]  id  The id of the scenario.
     */
    virtual async::task<void> on_parallel_scenario_started(uint32_t id);

    /**
     * @brief      Called when a parallel scenario is finished.
     *
     * @param[in]  id  The id of the scenario.
     */
    virtual async::task<void> on_parallel_scenario_finished(uint32_t id);

    /**
     * @brief      Executes a vector of scenarios in parallel.
     *
     *             This method is used to execute a vector of scenarios in parallel.
     *             The scenarios are executed in parallel and the result is returned.
     *
     * @param[in]  scenarios  The vector of scenarios to execute.
     *
     * @return     A task that completes when all scenarios are finished.
     */
    async::task<bool> parallel_scenarios(std::vector<std::pair<uint32_t, scenario_t>> scenarios);

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

    /**
     * @brief      Common hook handler for update external responses.
     *
     *             Checks if all bots are ready and notifies the controller when ready.
     */
    virtual async::task<void>
    on_hook_update_external(fb::bot::game_bot&                                         bot,
                            const fb::protocol::game::response::update_external<true>& response);

    /**
     * @brief      Sleeps for a specified duration.
     *
     * @param[in]  duration  The duration to sleep.
     *
     * @return     A task that completes when the sleep is finished.
     */
    async::task<void> sleep(std::chrono::milliseconds duration);

    /**
     * @brief      Arranges bots in line formation.
     *
     *             This method is used to arrange bots in line formation.
     *             The bots are arranged in a line formation and the result is returned.
     *
     * @return     A task that completes when the bots are arranged.
     */
    async::task<void> arrange_bots_in_line_formation();

    /**
     * @brief      Arranges bots in grid formation.
     *
     *             This method is used to arrange bots in a 2D grid formation.
     *             The bots are arranged in a grid from (start_x, start_y) to (end_x, end_y).
     *
     * @param[in]  start_x  The starting x coordinate for the grid.
     * @param[in]  start_y  The starting y coordinate for the grid.
     * @param[in]  end_x    The ending x coordinate for the grid.
     * @param[in]  end_y    The ending y coordinate for the grid.
     *
     * @return     A task that completes when the bots are arranged.
     */
    async::task<void>
    arrange_bots_in_grid_formation(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y);

    /**
     * @brief      Forms a group with all test bots.
     *
     *             This method creates a group by having the first bot invite all other bots
     *             to join the group. It sends group invitations to each bot and waits for
     *             confirmation responses.
     *
     * @return     A task that completes when the group formation is finished.
     *
     * @note       The first bot in the test bots list becomes the group leader
     * @note       All other bots are invited to join the group
     */
    async::task<void> form_group();

    /**
     * @brief      Cleans up group formation by disabling group options for all bots.
     *
     *             This method disables the group option for all test bots to ensure
     *             clean state after group-related tests. It sends update option requests
     *             to disable group functionality.
     *
     * @return     A task that completes when the group cleanup is finished.
     *
     * @note       This method should be called after group tests to reset bot states
     */
    async::task<void> cleanup_group();
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_TEST_CASE_H__