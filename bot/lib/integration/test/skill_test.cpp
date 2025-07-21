#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

skill_test::skill_test(game_bot_controller& controller) :
    bot_integration_test(controller, 6)
{ }

async::task<void> skill_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);

    auto bots = this->get_test_bots();
    fb::logger::debug("Starting skill test with {} bot", bots.size());

    if (bots.empty())
        throw std::runtime_error("No bots available for skill test");

    co_await super::arrange_bots_in_line_formation();
}

async::task<void> skill_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> skill_test::on_scenario_finished(uint32_t scenario_index)
{
    auto  bots   = this->get_test_bots();
    auto& caster = bots.front();
    auto  thread = caster->thread();

    co_await caster->change_level(5, DEFAULT_TIMEOUT);
    co_await caster->clear_all_spells(DEFAULT_TIMEOUT);
    co_await caster->clear_all_drop_items(DEFAULT_TIMEOUT);
    co_await thread->sleep(DEFAULT_INTERVAL);

    co_return;
}

async::task<void> skill_test::on_parallel_scenario_started(uint32_t id)
{
    auto  bots   = this->get_test_bots();
    auto& caster = bots[id];

    co_await caster->change_level(5, DEFAULT_TIMEOUT);
    co_return;
}

async::task<void> skill_test::on_parallel_scenario_finished(uint32_t id)
{
    auto  bots   = this->get_test_bots();
    auto& caster = bots[id];

    co_await caster->clear_all_spells(DEFAULT_TIMEOUT);
    co_await caster->clear_all_drop_items(DEFAULT_TIMEOUT);
    co_return;
}

async::task<bool> skill_test::scenario_1()
{
    auto bots      = this->get_test_bots();
    auto scenarios = std::vector<std::pair<uint32_t, scenario_t>>{
        {0,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_near_damage_spells(bots[0]);
         }},
        {0,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_attack_cast_spells(bots[0]);
         }},
        {0,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_multi_target_attack_cast_spells(bots[0]);
         }},
        {1,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_healing_spells(bots[1]);
         }},
        {1,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_loot_spell(bots[1]);
         }},
        {2,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_damage_spells(bots[2]);
         }},
        {2,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_near_target_damage_spells(bots[2]);
         }},
        {3,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_buff_debuff_spells(bots[3], bots[5]);
         }},
        {4,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_teleport_spells(bots[4], bots[5]);
         }},
        {4,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_disguise_spells(bots[4]);
         }},
        {4,
         [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_shout_spells(bots[4]);
         }},
    };
    co_return co_await this->parallel_scenarios(scenarios);
}

fb::generator<bot_integration_test::scenario_t> skill_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->scenario_1();
    };

    co_yield [this]() -> async::task<bool> {
        auto bots = this->get_test_bots();
        co_return co_await this->test_group_healing_spells(bots[0]);
    };

    co_yield [this]() -> async::task<bool> {
        auto bots = this->get_test_bots();
        co_return co_await this->test_area_damage_spells(bots[0]);
    };

    co_yield [this]() -> async::task<bool> {
        auto bots = this->get_test_bots();
        co_return co_await this->test_skill_cooldown_delays(bots[0]);
    };
}

std::string skill_test::name() const
{
    return "Skill Test";
}

} // namespace fb::bot::integration