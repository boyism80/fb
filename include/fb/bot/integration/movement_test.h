#ifndef __BOT_INTEGRATION_MOVEMENT_TEST_H__
#define __BOT_INTEGRATION_MOVEMENT_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <vector>
#include <memory>

namespace fb::bot::integration {

/**
 * @brief      Movement integration test implementation.
 *
 *             Tests bot movement by checking if all bots have non-zero oid IDs,
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
     * @brief      Initializes the movement test and spawns required bots.
     *
     *             Spawns 5 bots for movement testing and waits for them to connect.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     * @param[in]  endpoint    The server endpoint to connect bots to.
     *
     * @return     A task that completes when initialization is finished.
     */
    async::task<void> initialize(game_bot_controller& controller) override;

    /**
     * @brief      Executes the movement test.
     *
     *             Moves the last bot downward step by step with timing intervals.
     *             This method is called only when all bots are ready (have non-zero oid).
     *
     * @return     A task that completes when movement test finishes, returning true on success.
     */
    async::task<bool> execute() override;

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
     *             Movement test requires all bots to have received their oid IDs
     *             (oid != 0) before starting the test.
     *
     * @return     True if all spawned bots have non-zero oid, false otherwise.
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

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_MOVEMENT_TEST_H__