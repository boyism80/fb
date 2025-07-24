#ifndef __BOT_GAME_INTEGRATION_CONTROLLER_H__
#define __BOT_GAME_INTEGRATION_CONTROLLER_H__

#include <fb/bot/game_controller.h>
#include <fb/game/protocol.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/integration/test_case.h>
#include <fb/locker.h>
#include <fb/model/loader.h>
#include <memory>
#include <queue>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <shared_mutex>

namespace fb::bot::integration {

/**
 * @brief      Game bot bot_controller for integration testing.
 *
 *             This bot_controller implements sophisticated integration testing
 *             capabilities including scenario-based testing, response verification,
 *             and comprehensive test case management. Unlike load testing,
 *             it focuses on functional correctness and system behavior validation.
 *
 *             The controller manages test instances for lifetime management
 *             and coordinates their execution across multiple bots.
 */
class game_bot_controller : public fb::bot::game_bot_controller
{
private:
    using super = fb::bot::game_bot_controller;

private:
    std::vector<std::unique_ptr<bot_integration_test>> _test_instances; ///< Test instances for lifetime management
    std::queue<bot_integration_test*>                  _test_queue;     ///< Queue of tests to execute
    bot_integration_test* _current_test{nullptr};                       ///< Currently active test (non-owning pointer)

    // Test result tracking
    struct test_result
    {
        std::string name;    ///< Test name
        bool        success; ///< Whether the test passed
        std::string message; ///< Additional result message
    };
    std::vector<test_result> _test_results; ///< Results of completed tests

    // Hook system for integration tests - per test
    using hook_function = std::function<async::task<void>(game_bot&, const fb::protocol::header&)>;
    std::unordered_map<bot_integration_test*, std::unordered_map<uint8_t, std::vector<hook_function>>> _test_hooks;
    std::shared_mutex _hook_mutex; ///< Mutex for thread-safe hook operations

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
     * @brief      Notifies the controller that the current test is ready to start.
     *
     *             Called by test instances when they detect they are ready to begin execution.
     */
    void notify_test_ready();

    /**
     * @brief      Starts the current test.
     *
     *             Called when the current test notifies it is ready.
     */
    async::task<void> start_current_test();

    /**
     * @brief      Adds a test to the execution queue.
     *
     * @param[in]  test  Unique pointer to the test to add.
     */
    void enqueue_test(std::unique_ptr<bot_integration_test> test);

    /**
     * @brief      Starts the next test in the queue.
     *
     *             Called when the current test completes.
     */
    void start_next_test();

    /**
     * @brief      Checks if there are more tests in the queue.
     *
     * @return     True if there are more tests, false otherwise.
     */
    bool has_more_tests() const;

    /**
     * @brief      Prints the final test results summary.
     *
     *             Called when all tests have completed to show individual test results
     *             and overall success/failure status.
     */
    void print_final_test_results();

    /**
     * @brief      Activates the first test in the queue.
     *
     *             This method initializes the first test by creating bots,
     *             registering hooks, and performing other setup tasks.
     *             This is different from starting the test - activation
     *             prepares the test to become ready.
     */
    async::task<void> active_test();

    /**
     * @brief      Registers a hook for a specific protocol type for a test.
     *
     *             This method should be called by tests to register their hooks.
     *             Only hooks for the current test will be executed.
     *
     * @param[in]  test  Pointer to the test instance registering the hook.
     * @param[in]  fn    The hook function to register.
     *
     * @tparam     ResponseType  The protocol response type to hook.
     */
    template <typename ResponseType>
    void hook_for_test(bot_integration_test*                                                   test,
                       const std::function<async::task<void>(game_bot&, const ResponseType&)>& fn)
    {
        auto hook_func = [fn](game_bot& bot, const fb::protocol::header& header) -> async::task<void> {
            auto& protocol = static_cast<const ResponseType&>(header);
            co_await fn(bot, protocol);
        };

        auto unique_lock = std::unique_lock<std::shared_mutex>(this->_hook_mutex);
        this->_test_hooks[test][ResponseType::header].push_back(hook_func);
    }

    /**
     * @brief      Registers a member function as a hook for a specific protocol type from external class.
     *
     *             Allows external classes to register hooks by passing the class instance pointer.
     *
     * @param[in]  test      Pointer to the test instance registering the hook.
     * @param[in]  instance  Pointer to the class instance.
     * @param[in]  fn        The member function to register as hook.
     *
     * @tparam     Class         The class type containing the member function.
     * @tparam     ResponseType  The protocol response type to hook.
     */
    template <typename Class, typename ResponseType>
    void hook(bot_integration_test* test,
              Class*                instance,
              async::task<void> (Class::*fn)(game_bot&, const ResponseType&))
    {
        auto hook_func = [instance, fn](game_bot& bot, const fb::protocol::header& header) -> async::task<void> {
            auto& protocol = static_cast<const ResponseType&>(header);
            co_await (instance->*fn)(bot, protocol);
        };

        auto unique_lock = std::unique_lock<std::shared_mutex>(this->_hook_mutex);
        this->_test_hooks[test][ResponseType::header].push_back(hook_func);
    }

protected:
    /**
     * @brief      Overrides the base controller's integration hook execution.
     *
     *             This method executes hooks for the current test only.
     *
     * @param[in]  cmd     The command identifier.
     * @param[in]  bot     The bot that received the message.
     * @param[in]  header  The protocol header.
     *
     * @return     An async task that completes when hook processing is finished.
     */
    async::task<void> on_integration_hook_execution(uint8_t                     cmd,
                                                    game_bot&                   bot,
                                                    const fb::protocol::header& header) override;

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