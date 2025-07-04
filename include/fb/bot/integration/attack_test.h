#ifndef __BOT_INTEGRATION_ATTACK_TEST_H__
#define __BOT_INTEGRATION_ATTACK_TEST_H__

#include <fb/bot/integration/test_case.h>

namespace fb::bot::integration {

/**
 * @brief      Attack integration test implementation.
 *
 *             Tests bot attack functionality by spawning a single bot and
 *             performing 10 attack sequences with 1-second intervals.
 */
class attack_test : public bot_integration_test
{
private:
    bool _test_completed{false};
    bool _test_running{false};

    std::vector<std::shared_ptr<fb::bot::game_bot>> _test_bots; ///< Attack test's own bot collection

    static constexpr int ATTACK_COUNT = 5;

public:
    /**
     * @brief      Initializes the attack test and spawns a single bot.
     *
     *             Spawns 1 bot for attack testing and waits for it to connect.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     *
     * @return     A task that completes when initialization is finished.
     */
    async::task<void> initialize(game_bot_controller& controller) override;

    /**
     * @brief      Executes the attack test.
     *
     *             Performs 10 attack sequences with 1-second intervals using
     *             the spawned bot to test combat functionality.
     *
     * @return     A task that completes when attack test finishes, returning true on success.
     */
    async::task<bool> execute() override;

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
     *             Attack test requires the single bot to have received its oid ID
     *             (oid != 0) before starting the test.
     *
     * @return     True if the spawned bot has non-zero oid, false otherwise.
     */
    bool is_ready() const override;

    /**
     * @brief      Called when a bot connects to the attack test.
     *
     *             Adds the bot to the attack test's bot collection.
     *
     * @param[in]  bot  The connected bot to add to the test.
     */
    void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) override;

    /**
     * @brief      Cleans up attack test resources and disconnects the spawned bot.
     *
     *             Overrides base cleanup to handle attack test specific bot collection.
     */
    void cleanup() override;

protected:
    /**
     * @brief      Gets the current list of attack test bots (thread-safe).
     *
     * @return     Vector of shared pointers to the attack test bots.
     */
    std::vector<std::shared_ptr<fb::bot::game_bot>> get_test_bots() const;
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_ATTACK_TEST_H__