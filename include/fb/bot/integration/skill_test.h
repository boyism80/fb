#ifndef __BOT_INTEGRATION_SKILL_TEST_H__
#define __BOT_INTEGRATION_SKILL_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/bot/game_bot.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

/**
 * @brief      Skill integration test implementation.
 *
 *             Tests bot skill casting and coordination between multiple bots.
 */
class skill_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

private:
    // Test function queue management
    using test_function = std::function<async::task<bool>(std::vector<std::shared_ptr<fb::bot::game_bot>>&)>;
    std::vector<std::pair<std::string, test_function>> _test_functions;

    static constexpr int SPELL_CAST_COUNT = 10;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;
    async::task<void>     on_parallel_scenario_started(uint32_t id) override final;
    async::task<void>     on_parallel_scenario_finished(uint32_t id) override final;

public:
    /**
     * @brief      Constructs a new skill test with controller reference.
     *
     *             Initializes the skill test with 5 bots and registers common hooks.
     *
     * @param[in]  controller  Reference to the parent game bot controller.
     */
    skill_test(game_bot_controller& controller);

    /**
     * @brief      Gets the test name.
     *
     * @return     "Skill Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Executes the main skill test scenario.
     *
     *             This method orchestrates the complete skill testing sequence,
     *             including all spell categories and their respective test cases.
     *             It manages bot coordination and ensures proper test execution order.
     *
     * @return     A task that completes with true if all skill tests passed, false otherwise.
     *
     * @note       This is the primary entry point for skill testing execution
     */
    async::task<bool> scenario_1();

private:
    /**
     * @brief      Tests healing spells with comprehensive spell coverage.
     *
     *             Performs healing spell tests including:
     *             - Standard healing spells (self and target)
     *             - Dynamic healing spell (백호의희원)
     *             - Group healing spell (백호의희원'첨)
     *
     * @param[in]  caster   The bot casting the healing spells.
     *
     * @return     A task that completes when all healing spell tests finish.
     */
    async::task<bool> test_healing_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests damage spells with target-based damage calculation.
     *
     *             Performs damage spell tests including various target-based
     *             damage spells with proper damage calculation and verification.
     *
     * @param[in]  caster   The bot casting the damage spells.
     *
     * @return     A task that completes when all damage spell tests finish.
     */
    async::task<bool> test_damage_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests near damage spells with area-based damage calculation.
     *
     *             Performs near damage spell tests including area-based
     *             damage spells that affect nearby targets.
     *
     * @param[in]  caster   The bot casting the near damage spells.
     *
     * @return     A task that completes when all near damage spell tests finish.
     */
    async::task<bool> test_near_damage_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests attack-cast spells with dynamic effect calculation.
     *
     *             Performs attack-cast spell tests including spells that
     *             combine attack mechanics with spell casting effects.
     *
     * @param[in]  caster   The bot casting the attack-cast spells.
     *
     * @return     A task that completes when all attack-cast spell tests finish.
     */
    async::task<bool> test_attack_cast_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests near target damage spells with proximity-based targeting.
     *
     *             Performs near target damage spell tests including spells that
     *             target nearby enemies with damage effects.
     *
     * @param[in]  caster   The bot casting the near target damage spells.
     *
     * @return     A task that completes when all near target damage spell tests finish.
     */
    async::task<bool> test_near_target_damage_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests area damage spells with wide-range effect calculation.
     *
     *             Performs area damage spell tests including spells that
     *             affect multiple targets in a wide area.
     *
     * @param[in]  caster   The bot casting the area damage spells.
     *
     * @return     A task that completes when all area damage spell tests finish.
     */
    async::task<bool> test_area_damage_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests buff and debuff spells with status effect verification.
     *
     *             Performs buff and debuff spell tests including spells that
     *             apply positive or negative status effects to targets.
     *
     * @param[in]  caster   The bot casting the buff/debuff spells.
     * @param[in]  target   The target bot for buff/debuff spell tests.
     *
     * @return     A task that completes when all buff/debuff spell tests finish.
     */
    async::task<bool> test_buff_debuff_spells(std::shared_ptr<fb::bot::game_bot> caster,
                                              std::shared_ptr<fb::bot::game_bot> target);

    /**
     * @brief      Tests target spells with pre and post condition verification.
     *
     *             Performs target spell tests including spells that require specific
     *             conditions before casting and verify effects after casting.
     *             These spells use 'you' parameter and are typically one-time effects.
     *
     * @param[in]  caster   The bot casting the target spells.
     * @param[in]  target   The target bot for target spell tests.
     *
     * @return     A task that completes when all target spell tests finish.
     */
    async::task<bool> test_target_spells(std::shared_ptr<fb::bot::game_bot> caster,
                                         std::shared_ptr<fb::bot::game_bot> target);

    /**
     * @brief      Tests special spells with complex requirements and effects.
     *
     *             Performs special spell tests including teleportation, weapon damage,
     *             buffs, summoning, and ghost-specific spells. These spells often
     *             require specific conditions, message inputs, or state changes.
     *
     * @param[in]  caster   The bot casting the special spells.
     *
     * @return     A task that completes when all special spell tests finish.
     */
    async::task<bool> test_special_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests multi-target attack-cast spells with complex effect calculation.
     *
     *             Performs multi-target attack-cast spell tests including spells that
     *             affect multiple targets with combined attack and spell effects.
     *
     * @param[in]  caster   The bot casting the multi-target attack-cast spells.
     *
     * @return     A task that completes when all multi-target attack-cast spell tests finish.
     */
    async::task<bool> test_multi_target_attack_cast_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests teleport spells (출두, 소환) with map movement scenarios.
     *
     * @param[in]  caster   The bot casting the teleport spells.
     * @param[in]  target   The target bot for teleport spell tests.
     *
     * @return     An async task that completes when teleport testing is finished.
     */
    async::task<bool> test_teleport_spells(std::shared_ptr<fb::bot::game_bot> caster,
                                           std::shared_ptr<fb::bot::game_bot> target);

    /**
     * @brief      Tests disguise spells (경수, 맹수, 야수, 금수) with all available monster transformations.
     *
     * @param[in]  caster   The bot casting the disguise spells.
     *
     * @return     An async task that completes when disguise testing is finished.
     */
    async::task<bool> test_disguise_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests shout spells (사자후전사, 사자후도사, 사자후술사, 사자후도적) with message input and SHOUT
     * verification.
     *
     * @param[in]  caster   The bot casting the shout spells.
     *
     * @return     An async task that completes when shout testing is finished.
     */
    async::task<bool> test_shout_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests loot spell (노획) with item and money loot scenarios.
     *
     * @param[in]  caster   The bot casting the loot spell.
     *
     * @return     An async task that completes when loot testing is finished.
     */
    async::task<bool> test_loot_spell(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Tests group healing spells with comprehensive group management.
     *
     *             Performs group healing spell tests including:
     *             - Group formation and management
     *             - Dynamic MP-based healing (백호의희원'첨)
     *             - Fixed-value group healing (신령의기원'첨)
     *             - Group cleanup after testing
     *
     * @param[in]  caster   The bot casting the group healing spells.
     *
     * @return     A task that completes when all group healing spell tests finish.
     */
    async::task<bool> test_group_healing_spells(std::shared_ptr<fb::bot::game_bot> caster);

    /**
     * @brief      Verifies that group healing effects were applied correctly to all group members.
     *
     * @param[in]  bots              The list of bots to verify.
     * @param[in]  before_hp_values  The HP values before casting the group healing spell.
     * @param[in]  expected_hp_gain  The expected HP gain from the group healing spell.
     *
     * @return     A task that completes when verification is finished.
     */
    async::task<bool> verify_group_healing_effects(const std::vector<int>& before_hp_values, int expected_hp_gain);

    /**
     * @brief      Tests the 출두 (teleport to target) spell.
     *
     *             Moves caster to different map, then casts 출두 to teleport to target.
     *
     * @param[in]  caster           The bot casting the spell.
     * @param[in]  target           The target bot to teleport to.
     * @param[in]  spell_slot       The spell slot index.
     * @param[in]  expected_mp_cost The expected MP cost for the spell.
     *
     * @return     A task that completes when 출두 test is finished.
     */
    async::task<void> test_chuldu_spell(std::shared_ptr<fb::bot::game_bot> caster,
                                        std::shared_ptr<fb::bot::game_bot> target,
                                        uint8_t                            spell_slot,
                                        int                                expected_mp_cost);

    /**
     * @brief      Tests the 소환 (summon target) spell.
     *
     *             Moves target to different map, then casts 소환 to summon target to caster.
     *
     * @param[in]  caster           The bot casting the spell.
     * @param[in]  target           The target bot to summon.
     * @param[in]  spell_slot       The spell slot index.
     * @param[in]  expected_mp_cost The expected MP cost for the spell.
     *
     * @return     A task that completes when 소환 test is finished.
     */
    async::task<void> test_sohwan_spell(std::shared_ptr<fb::bot::game_bot> caster,
                                        std::shared_ptr<fb::bot::game_bot> target,
                                        uint8_t                            spell_slot,
                                        int                                expected_mp_cost);

    /**
     * @brief      Tests skill cooldown delays with comprehensive cooldown verification.
     *
     *             Performs cooldown tests including:
     *             - Hellfire spell cooldown testing
     *             - Cooldown reduction verification based on level and equipment
     *             - Cooldown message parsing and validation
     *
     * @param[in]  caster   The bot casting the spells for cooldown testing.
     *
     * @return     A task that completes with true if all cooldown tests passed, false otherwise.
     */
    async::task<bool> test_skill_cooldown_delays(std::shared_ptr<fb::bot::game_bot> caster);

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
     * @brief      Structure for near target damage spell test data
     */
    struct near_target_damage_spell_test
    {
        std::string                                                                   name;
        SPELL_TYPE                                                                    type;
        std::function<std::pair<int, int>(const std::shared_ptr<fb::bot::game_bot>&)> calculator;
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
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_SKILL_TEST_H__