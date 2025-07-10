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

protected:
    game_bot_controller&                            _controller; ///< Reference to the parent game bot controller
    std::vector<std::shared_ptr<fb::bot::game_bot>> _test_bots;  ///< Collection of bots managed by this test
    test_state                                      _state{test_state::idle}; ///< Current state of the test

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
     */
    bot_integration_test(game_bot_controller& controller) :
        _controller(controller)
    { }

    virtual ~bot_integration_test() = default;

    /**
     * @brief      Gets the current state of the test.
     *
     * @return     The current test state.
     */
    test_state get_state() const
    {
        return this->_state;
    }

    /**
     * @brief      Sets the current state of the test.
     *
     * @param[in]  state  The new test state.
     */
    void set_state(test_state state)
    {
        this->_state = state;
    }

    /**
     * @brief      Checks if the test has completed execution.
     *
     * @return     True if the test is complete, false otherwise.
     */
    bool is_complete() const
    {
        return this->_state == test_state::completed;
    }

    /**
     * @brief      Checks if the test is currently running.
     *
     * @return     True if the test is running, false otherwise.
     */
    bool is_running() const
    {
        return this->_state == test_state::running;
    }

    /**
     * @brief      Called when a bot connects to the test.
     *
     *             Default implementation adds the bot to the test's bot collection.
     *             Can be overridden by derived classes for custom behavior.
     *
     * @param[in]  bot  Shared pointer to the connected bot.
     */
    virtual void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
    {
        this->_test_bots.push_back(bot);
    }

    /**
     * @brief      Notifies the controller that this test is ready to start.
     *
     *             Called by derived classes when they detect they are ready to begin execution.
     */
    void notify_ready();

    /**
     * @brief      Performs cleanup operations when the test is destroyed.
     *
     *             Default implementation does nothing. Can be overridden
     *             by derived classes for custom cleanup logic.
     */
    virtual void cleanup()
    {
        for (auto bot : this->get_test_bots())
        {
            if (bot)
                bot->close();
        }

        fb::logger::info("{} test cleanup completed - all bots disconnected", this->name());
    }

    /**
     * @brief      Checks if the test is ready to start execution.
     *
     *             This method should be overridden by derived classes to
     *             implement specific readiness checks.
     *
     * @return     True if the test is ready to start, false otherwise.
     */
    virtual bool is_ready() const
    {
        // Default implementation: ready if we have at least one bot
        return !this->_test_bots.empty();
    }

    /**
     * @brief      Gets the current list of test bots.
     *
     * @return     Vector of shared pointers to the test bots.
     */
    std::vector<std::shared_ptr<fb::bot::game_bot>> get_test_bots() const
    {
        return this->_test_bots;
    }

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
    virtual void reset()
    {
        this->_state = test_state::idle;
    }
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_TEST_CASE_H__