#ifndef __BOT_INTEGRATION_SKILL_TEST_H__
#define __BOT_INTEGRATION_SKILL_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/bot/game_bot.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

class skill_test : public bot_integration_test
{
private:
    using super         = bot_integration_test;
    using test_function = std::function<async::task<bool>(std::vector<std::shared_ptr<fb::bot::game_bot>>&)>;

    struct healing_spell_test
    {
        std::string name;
        SPELL_TYPE  type;
        int         expected_hp_gain;
        int         expected_mp_cost;
    };

    struct damage_spell_test
    {
        std::string name;
        SPELL_TYPE  type;
        int         expected_damage;
        int         expected_mp_cost;
    };

    struct near_damage_spell_test
    {
        std::string name;
        SPELL_TYPE  type;
        int         expected_damage;
        int         expected_mp_cost;
    };

    struct near_target_damage_spell_test
    {
        std::string                                                                   name;
        SPELL_TYPE                                                                    type;
        std::function<std::pair<int, int>(const std::shared_ptr<fb::bot::game_bot>&)> calculator;
    };

    using spell_calculator = std::function<std::pair<int, int>(const std::shared_ptr<fb::bot::game_bot>&)>;
    struct attack_cast_spell_test
    {
        std::string      name;
        SPELL_TYPE       type;
        spell_calculator calculator;
        bool             has_preprocess;
    };

    using multi_target_spell_calculator = std::function<std::tuple<int, int, std::optional<fb::model::point<uint16_t>>>(const std::shared_ptr<fb::bot::game_bot>&)>;

    struct multi_target_attack_cast_spell_test
    {
        std::string                      name;
        SPELL_TYPE                       type;
        multi_target_spell_calculator    calculator;
        std::vector<std::pair<int, int>> spawn_positions;
        bool                             has_movement;
    };

private:
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
    skill_test(game_bot_controller& controller);
    std::string name() const override final;

private:
    async::task<bool> scenario_1();

private:
    async::task<bool> test_healing_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_damage_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_near_damage_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_attack_cast_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_near_target_damage_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_area_damage_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_buff_debuff_spells(std::shared_ptr<fb::bot::game_bot> caster, std::shared_ptr<fb::bot::game_bot> target);
    async::task<bool> test_target_spells(std::shared_ptr<fb::bot::game_bot> caster, std::shared_ptr<fb::bot::game_bot> target);
    async::task<bool> test_special_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_multi_target_attack_cast_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_teleport_spells(std::shared_ptr<fb::bot::game_bot> caster, std::shared_ptr<fb::bot::game_bot> target);
    async::task<bool> test_disguise_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_shout_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_loot_spell(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> test_group_healing_spells(std::shared_ptr<fb::bot::game_bot> caster);
    async::task<bool> verify_group_healing_effects(const std::vector<int>& before_hp_values, int expected_hp_gain);
    async::task<void> test_chuldu_spell(std::shared_ptr<fb::bot::game_bot> caster, std::shared_ptr<fb::bot::game_bot> target, uint8_t spell_slot, int expected_mp_cost);
    async::task<void> test_sohwan_spell(std::shared_ptr<fb::bot::game_bot> caster, std::shared_ptr<fb::bot::game_bot> target, uint8_t spell_slot, int expected_mp_cost);
    async::task<bool> test_skill_cooldown_delays(std::shared_ptr<fb::bot::game_bot> caster);
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_SKILL_TEST_H__