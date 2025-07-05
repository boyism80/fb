#ifndef __BOT_INTEGRATION_SKILL_TEST_H__
#define __BOT_INTEGRATION_SKILL_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <stdexcept>

namespace fb::bot::integration {

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

    std::vector<std::shared_ptr<fb::bot::game_bot>> _test_bots; ///< Skill test's own bot collection

    // Spell learning tracking
    bool    _waiting_for_spell_update{false};
    uint8_t _spell_learned_index{0};

    static constexpr int SPELL_CAST_COUNT = 10;

public:
    /**
     * @brief      Initializes the skill test and spawns a single bot.
     *
     *             Spawns 1 bot for skill testing and waits for it to connect.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     *
     * @return     A task that completes when initialization is finished.
     */
    async::task<void> initialize(game_bot_controller& controller) override;

    /**
     * @brief      Executes the skill test.
     *
     *             Tests bot skill casting functionality by sending a spell learning chat,
     *             waiting for spell_update response, then casting the learned spell 10 times.
     *
     * @return     A task that completes when skill test finishes, returning true on success.
     */
    async::task<bool> execute() override;

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
     *             Skill test requires all bots to have received their oid IDs
     *             and be in proper spell-casting ready state before starting.
     *
     * @return     True if all spawned bots are ready for skill test, false otherwise.
     */
    bool is_ready() const override;

    /**
     * @brief      Called when a bot connects to the skill test.
     *
     *             Stores the connected bot for skill testing and sets up protocol handlers.
     *
     * @param[in]  bot  The connected bot to store for testing.
     */
    void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) override;

    /**
     * @brief      Handles spell_update response from the server.
     *
     *             Called when a bot receives a spell_update packet after learning a spell.
     *             Stores the spell index for subsequent spell casting.
     *
     * @param[in]  bot    The bot that received the spell_update.
     * @param[in]  index  The index of the newly learned spell.
     */
    void on_spell_update_received(std::shared_ptr<fb::bot::game_bot> bot, uint8_t index);

private:
    /**
     * @brief      Gets the list of connected test bots.
     *
     * @return     Vector of test bots for skill testing.
     */
    const std::vector<std::shared_ptr<fb::bot::game_bot>>& get_test_bots() const
    {
        return this->_test_bots;
    }

    /**
     * @brief      Tests healing spells with comprehensive spell coverage.
     *
     *             Performs healing spell tests including:
     *             - Standard healing spells (self and target)
     *             - Dynamic healing spell (백호의희원)
     *             - Group healing spell (백호의희원'첨)
     *
     * @param[in]  bots     The list of bots to use for testing.
     * @param[in]  timeout  The timeout for each spell operation.
     *
     * @return     A task that completes when all healing spell tests finish.
     */
    async::task<bool> test_healing_spells(const std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                          std::chrono::milliseconds                              timeout);

    async::task<bool> test_damage_spells(const std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                         std::chrono::milliseconds                              timeout);
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_SKILL_TEST_H__