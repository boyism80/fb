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

    // Test function queue management
    using test_function = std::function<async::task<bool>(const std::vector<std::shared_ptr<fb::bot::game_bot>>&,
                                                          std::chrono::milliseconds)>;
    std::vector<std::pair<std::string, test_function>> _test_functions;

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
     * @brief      Initializes the test function queue with all test functions.
     *
     *             Registers all test functions in the order they should be executed.
     */
    void initialize_test_functions();

    /**
     * @brief      Executes all registered test functions in sequence.
     *
     * @param[in]  bots     The list of bots to use for testing.
     * @param[in]  timeout  The timeout for each test operation.
     *
     * @return     A task that completes when all test functions finish.
     */
    async::task<bool> execute_test_functions(const std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                             std::chrono::milliseconds                              timeout);

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

    async::task<bool> test_near_damage_spells(const std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                              std::chrono::milliseconds                              timeout);

    async::task<bool> test_attack_cast_spells(const std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                              std::chrono::milliseconds                              timeout);

    async::task<bool> test_multi_target_attack_cast_spells(const std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                                           std::chrono::milliseconds timeout);

    /**
     * @brief      Structure for healing spell test data
     */
    struct healing_spell_test
    {
        std::string name;             ///< Spell name
        SPELL_TYPE  type;             ///< Spell type (NORMAL or TARGET)
        int         expected_hp_gain; ///< Expected HP gain from the spell
        int         expected_mp_cost; ///< Expected MP cost for casting the spell
    };

    /**
     * @brief      Structure for damage spell test data
     */
    struct damage_spell_test
    {
        std::string name;             ///< Spell name
        SPELL_TYPE  type;             ///< Spell type (TARGET)
        int         expected_damage;  ///< Expected damage dealt by the spell
        int         expected_mp_cost; ///< Expected MP cost for casting the spell
    };

    /**
     * @brief      Structure for near damage spell test data
     */
    struct near_damage_spell_test
    {
        std::string name;             ///< Spell name
        SPELL_TYPE  type;             ///< Spell type (NORMAL)
        int         expected_damage;  ///< Expected damage dealt by the spell
        int         expected_mp_cost; ///< Expected MP cost for casting the spell
    };

    /**
     * @brief      Function type for calculating expected HP/MP changes for attack_cast spells
     */
    using spell_calculator = std::function<std::pair<int, int>(const std::shared_ptr<fb::bot::game_bot>&)>;

    /**
     * @brief      Structure for attack_cast spell test data
     */
    struct attack_cast_spell_test
    {
        std::string      name;           ///< Spell name
        SPELL_TYPE       type;           ///< Spell type (NORMAL)
        spell_calculator calculator;     ///< Function to calculate expected HP/MP changes
        bool             has_preprocess; ///< Whether the spell has preprocess (for buff spells)
    };

    /**
     * @brief      Function type for calculating expected HP/MP/position changes for multi-target attack_cast spells
     */
    using multi_target_spell_calculator = std::function<std::tuple<int, int, std::optional<fb::model::point<uint16_t>>>(
        const std::shared_ptr<fb::bot::game_bot>&)>;

    /**
     * @brief      Structure for multi-target attack_cast spell test data
     */
    struct multi_target_attack_cast_spell_test
    {
        std::string                      name;            ///< Spell name
        SPELL_TYPE                       type;            ///< Spell type (NORMAL)
        multi_target_spell_calculator    calculator;      ///< Function to calculate expected HP/MP/position changes
        std::vector<std::pair<int, int>> spawn_positions; ///< Relative positions from caster for monster spawning
        bool                             has_movement;    ///< Whether the spell moves the caster
    };

    /**
     * @brief      Structure for spawned monster information
     */
    struct spawned_monster_info
    {
        uint32_t                   oid;      ///< Monster object ID
        fb::model::point<uint16_t> position; ///< Monster position
        uint32_t                   look;     ///< Monster look value
    };
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_SKILL_TEST_H__