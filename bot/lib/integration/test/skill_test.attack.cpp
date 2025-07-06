#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_attack_cast_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                                      std::chrono::milliseconds                        timeout)
{
    constexpr auto interval = 100ms;

    auto& caster = bots.at(0);
    auto& target = bots.at(1);

    fb::logger::info("Bot {} starting attack_cast spell test", caster->oid());
    caster->chat("=== ATTACK_CAST SPELL TEST STARTED ===");

    auto thread = caster->thread();
    co_await thread->switching();

    // Setup all bots with max HP/MP
    for (auto& bot : bots)
    {
        std::ignore = co_await this->setup_bot_stats(bot, 100000, 100000, std::nullopt, std::nullopt, timeout);
    }

    std::vector<attack_cast_spell_test> attack_cast_spells = {
        // Single target attack spells (front target)
        {"건곤대나이",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp * 2 / 3; // hp = me:hp()*2 // 3
             return {current_hp - hp_cost, current_mp - 40};
         }, false},

        {"동귀어진",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp - 10; // hp = me:hp() - 10
             return {current_hp - hp_cost, current_mp - 10};
         }, false},

        {"백호검무",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2; // hp = me:hp() / 2
             return {current_hp - hp_cost, current_mp - 60};
         }, false},

        {"백호참",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2; // hp = me:hp() / 2
             return {current_hp - hp_cost, current_mp - 60};
         }, false},

        {"필살검무",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2;                // hp = me:hp() / 2
             auto mp_cost    = std::max<int>(current_mp, 60); // mp = max(current_mp, 60)
             return {current_hp - hp_cost, current_mp - mp_cost};
         }, false},

        {"무형검",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2; // hp = me:hp() / 2
             return {current_hp - hp_cost, current_mp - 180};
         }, true },

        {"이기어검",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2; // hp = me:hp() / 2
             return {current_hp - hp_cost, current_mp - 180};
         }, true }
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : attack_cast_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await this->learn_spells(caster, spell_names, timeout);
    fb::logger::info("Successfully learned {} out of {} attack_cast spells", learned_count, attack_cast_spells.size());

    fb::logger::info("Learning {} attack_cast spells", attack_cast_spells.size());
    auto spell_slot = 1;

    for (const auto& spell : attack_cast_spells)
    {
        fb::logger::info("Testing spell: {}", spell.name);

        // Spawn a monster in front of the caster
        std::ignore = co_await this->spawn_monster_relative_by_look(caster, "다람쥐", 0, 1, 32793, timeout);

        // Set caster's current HP/MP for testing
        std::ignore = co_await this->set_current_hp_mp(caster, 1000, 1000, timeout);

        // Calculate expected values using the spell calculator function
        auto [expected_hp, expected_mp] = spell.calculator(caster);

        // Cast the spell
        co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", 0, {0, 0}),
            [=](auto& resp) -> bool {
                return resp.ch_hp == expected_hp && resp.ch_mp == expected_mp;
            },
            timeout);

        spell_slot++;
        co_await caster->thread()->sleep(interval);
    }

    caster->chat("=== ATTACK_CAST SPELL TEST COMPLETED ===");
    fb::logger::info("Attack_cast spell test completed.");
    co_return true;
}
