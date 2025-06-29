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
     * @brief      Spawns the required number of bots for this test.
     *
     *             Each test implementation defines how many bots it needs
     *             and spawns them using the provided controller.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     * @param[in]  endpoint    The server endpoint to connect bots to.
     *
     * @return     A task that completes when all required bots are spawned.
     */
    virtual async::task<void> spawn_bots(game_bot_controller&                  controller,
                                         const boost::asio::ip::tcp::endpoint& endpoint) = 0;

    /**
     * @brief      Executes the integration test with the spawned bots.
     *
     *             This method is called after all bots are ready and starts
     *             the actual test execution logic.
     *
     * @return     A task that completes when the test finishes.
     */
    virtual async::task<void> execute() = 0;

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
     *             For example, movement test might require all bots to have non-zero sequence,
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

/**
 * @brief      Movement integration test implementation.
 *
 *             Tests bot movement by checking if all bots have non-zero sequence IDs,
 *             then moving the last bot downward by 5 positions with 1-second intervals.
 */
class movement_test : public bot_integration_test
{
private:
    bool _test_started{false};
    bool _test_completed{false};
    bool _test_running{false};

    std::vector<std::shared_ptr<fb::bot::game_bot>> _test_bots; ///< Movement test's own bot collection

    static constexpr int MOVEMENT_STEPS = 5;

public:
    /**
     * @brief      Spawns the required bots for movement test.
     *
     *             Spawns 5 bots for movement testing and waits for them to connect.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     * @param[in]  endpoint    The server endpoint to connect bots to.
     *
     * @return     A task that completes when all bots are spawned.
     */
    async::task<void> spawn_bots(game_bot_controller&                  controller,
                                 const boost::asio::ip::tcp::endpoint& endpoint) override;

    /**
     * @brief      Executes the movement test.
     *
     *             Moves the last bot downward step by step with timing intervals.
     *             This method is called only when all bots are ready (have non-zero sequence).
     *
     * @return     A task that completes when movement test finishes.
     */
    async::task<void> execute() override;

    /**
     * @brief      Checks if the movement test has completed.
     *
     * @return     True if all movement steps are finished.
     */
    bool is_complete() const override
    {
        return this->_test_completed;
    }

    /**
     * @brief      Resets the movement test state.
     */
    void reset() override;

    /**
     * @brief      Gets the test name.
     *
     * @return     "Movement Test" as the identifier.
     */
    std::string name() const override
    {
        return "Movement Test";
    }

    /**
     * @brief      Checks if the movement test is currently running.
     *
     * @return     True if the test is in progress.
     */
    bool is_running() const override
    {
        return this->_test_running;
    }

    /**
     * @brief      Checks if all spawned bots are ready for movement test.
     *
     *             Movement test requires all bots to have received their sequence IDs
     *             (sequence != 0) before starting the test.
     *
     * @return     True if all spawned bots have non-zero sequence, false otherwise.
     */
    bool is_ready() const override;

    /**
     * @brief      Called when a bot connects to the movement test.
     *
     *             Adds the bot to the movement test's bot collection.
     *
     * @param[in]  bot  The connected bot to add to the test.
     */
    void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) override;

    /**
     * @brief      Cleans up movement test resources and disconnects all spawned bots.
     *
     *             Overrides base cleanup to handle movement test specific bot collection.
     */
    void cleanup() override;

protected:
    /**
     * @brief      Gets the current list of movement test bots (thread-safe).
     *
     * @return     Vector of shared pointers to the movement test bots.
     */
    std::vector<std::shared_ptr<fb::bot::game_bot>> get_test_bots() const;
};

/**
 * @brief      Attack integration test implementation.
 *
 *             Tests bot attack functionality by coordinating attack sequences
 *             between multiple bots.
 */
class attack_test : public bot_integration_test
{
private:
    bool _test_completed{false};
    bool _test_running{false};

public:
    /**
     * @brief      Spawns the required bots for attack test.
     *
     *             Spawns the required number of bots for attack testing.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     * @param[in]  endpoint    The server endpoint to connect bots to.
     *
     * @return     A task that completes when all bots are spawned.
     */
    async::task<void> spawn_bots(game_bot_controller&                  controller,
                                 const boost::asio::ip::tcp::endpoint& endpoint) override;

    /**
     * @brief      Executes the attack test.
     *
     *             Coordinates attack sequences between multiple bots to test
     *             combat functionality and bot interaction during attacks.
     *
     * @return     A task that completes when attack test finishes.
     */
    async::task<void> execute() override;

    /**
     * @brief      Checks if the attack test has completed.
     *
     * @return     True if all attack sequences are finished.
     */
    bool is_complete() const override
    {
        return this->_test_completed;
    }

    /**
     * @brief      Resets the attack test state to initial conditions.
     */
    void reset() override;

    /**
     * @brief      Gets the attack test name.
     *
     * @return     "Attack Test" as the identifier.
     */
    std::string name() const override
    {
        return "Attack Test";
    }

    /**
     * @brief      Checks if the attack test is currently running.
     *
     * @return     True if the test is in progress.
     */
    bool is_running() const override
    {
        return this->_test_running;
    }

    /**
     * @brief      Checks if all spawned bots are ready for attack test.
     *
     *             Attack test requires all bots to have received their sequence IDs
     *             and be in proper combat-ready state before starting.
     *
     * @return     True if all spawned bots are ready for attack test, false otherwise.
     */
    bool is_ready() const override;

    /**
     * @brief      Called when a bot connects to the attack test.
     *
     *             Attack test currently doesn't store individual bots.
     *
     * @param[in]  bot  The connected bot (ignored for now).
     */
    void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) override;
};

/**
 * @brief      Skill integration test implementation.
 *
 *             Tests bot skill casting and coordination between multiple bots.
 */
class skill_test : public bot_integration_test
{
private:
    bool _test_completed{false};
    bool _test_running{false};

public:
    /**
     * @brief      Spawns the required bots for skill test.
     *
     *             Spawns the required number of bots for skill testing.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     * @param[in]  endpoint    The server endpoint to connect bots to.
     *
     * @return     A task that completes when all bots are spawned.
     */
    async::task<void> spawn_bots(game_bot_controller&                  controller,
                                 const boost::asio::ip::tcp::endpoint& endpoint) override;

    /**
     * @brief      Executes the skill test.
     *
     *             Tests bot skill casting functionality by coordinating spell
     *             casting sequences and skill interactions between multiple bots.
     *
     * @return     A task that completes when skill test finishes.
     */
    async::task<void> execute() override;

    /**
     * @brief      Checks if the skill test has completed.
     *
     * @return     True if all skill casting sequences are finished.
     */
    bool is_complete() const override
    {
        return this->_test_completed;
    }

    /**
     * @brief      Resets the skill test state to initial conditions.
     */
    void reset() override;

    /**
     * @brief      Gets the skill test name.
     *
     * @return     "Skill Test" as the identifier.
     */
    std::string name() const override
    {
        return "Skill Test";
    }

    /**
     * @brief      Checks if the skill test is currently running.
     *
     * @return     True if the test is in progress.
     */
    bool is_running() const override
    {
        return this->_test_running;
    }

    /**
     * @brief      Checks if all spawned bots are ready for skill test.
     *
     *             Skill test requires all bots to have received their sequence IDs
     *             and be in proper spell-casting ready state before starting.
     *
     * @return     True if all spawned bots are ready for skill test, false otherwise.
     */
    bool is_ready() const override;

    /**
     * @brief      Called when a bot connects to the skill test.
     *
     *             Skill test currently doesn't store individual bots.
     *
     * @param[in]  bot  The connected bot (ignored for now).
     */
    void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) override;
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_TEST_CASE_H__