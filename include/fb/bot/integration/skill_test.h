#ifndef __BOT_INTEGRATION_SKILL_TEST_H__
#define __BOT_INTEGRATION_SKILL_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <stdexcept>
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
     * @brief      Tests healing spells with comprehensive spell coverage.
     *
     *             Performs healing spell tests including:
     *             - Standard healing spells (self and target)
     *             - Dynamic healing spell (백호의희원)
     *             - Group healing spell (백호의희원'첨)
     *
     * @param[in]  bots     The list of bots to use for testing.
     *
     * @return     A task that completes when all healing spell tests finish.
     */
    async::task<bool> test_healing_spells();

    async::task<bool> test_damage_spells();

    async::task<bool> test_near_damage_spells();

    async::task<bool> test_attack_cast_spells();

    async::task<bool> test_near_target_damage_spells();

    async::task<bool> test_area_damage_spells();

    async::task<bool> test_buff_debuff_spells();

    async::task<bool> test_multi_target_attack_cast_spells();

    /**
     * @brief      Tests teleport spells (출두, 소환) with map movement scenarios.
     *
     * @param[in]  bots     The bot instances to use for testing.
     *
     * @return     An async task that completes when teleport testing is finished.
     */
    async::task<bool> test_teleport_spells();

    /**
     * @brief      Tests disguise spells (경수, 맹수, 야수, 금수) with all available monster transformations.
     *
     * @param[in]  bots     The bot instances to use for testing.
     *
     * @return     An async task that completes when disguise testing is finished.
     */
    async::task<bool> test_disguise_spells();

    /**
     * @brief      Tests shout spells (사자후전사, 사자후도사, 사자후술사, 사자후도적) with message input and SHOUT
     * verification.
     *
     * @param[in]  bots     The bot instances to use for testing.
     *
     * @return     An async task that completes when shout testing is finished.
     */
    async::task<bool> test_shout_spells();

    /**
     * @brief      Tests loot spell (노획) with item and money loot scenarios.
     *
     * @param[in]  bots     The bot instances to use for testing.
     *
     * @return     An async task that completes when loot testing is finished.
     */
    async::task<bool> test_loot_spell();

    /**
     * @brief      Tests group healing spells with comprehensive group management.
     *
     *             Performs group healing spell tests including:
     *             - Group formation and management
     *             - Dynamic MP-based healing (백호의희원'첨)
     *             - Fixed-value group healing (신령의기원'첨)
     *             - Group cleanup after testing
     *
     * @param[in]  bots     The list of bots to use for testing.
     *
     * @return     A task that completes when all group healing spell tests finish.
     */
    async::task<bool> test_group_healing_spells();

    /**
     * @brief      Forms a group with all provided bots.
     *
     *             The first bot becomes the group leader and invites all other bots.
     *
     * @param[in]  bots     The list of bots to form into a group.
     *
     * @return     A task that completes when group formation is finished.
     */
    async::task<void> form_group();

    /**
     * @brief      Prepares all bots for group healing test by setting appropriate HP levels.
     *
     *             Sets bots to low HP values to ensure visible healing effects.
     *             Adjusts HP based on expected healing amount to prevent overflow.
     *
     * @param[in]  bots              The list of bots to prepare.
     * @param[in]  expected_hp_gain  The expected HP gain from the group healing spell.
     *
     * @return     A task that completes when all bots are prepared.
     */
    async::task<void> prepare_bots_for_group_healing(int expected_hp_gain);

    /**
     * @brief      Verifies that group healing effects were applied correctly to all group members.
     *
     * @param[in]  bots              The list of bots to verify.
     * @param[in]  before_hp_values  The HP values before casting the group healing spell.
     * @param[in]  expected_hp_gain  The expected HP gain from the group healing spell.
     *
     * @return     A task that completes when verification is finished.
     */
    async::task<void> verify_group_healing_effects(const std::vector<int>& before_hp_values, int expected_hp_gain);

    /**
     * @brief      Cleans up group formation by removing all bots from the group.
     *
     * @param[in]  bots     The list of bots to remove from the group.
     *
     * @return     A task that completes when group cleanup is finished.
     */
    async::task<void> cleanup_group();

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