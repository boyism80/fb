#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_near_target_damage_spells()
{
    auto  bots   = this->get_test_bots();
    auto& caster = bots.front();

    fb::logger::info("Bot {} starting near target damage spell test", caster->oid());
    caster->chat("=== NEAR TARGET DAMAGE SPELL TEST STARTED ===");

    auto thread = caster->thread();
    co_await thread->switching();

    // Setup all bots with max HP/MP
    for (auto& bot : bots)
    {
        std::ignore = co_await bot->setup_bot_stats(100000, 100000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);
    }

    auto near_target_spells = std::vector<near_target_damage_spell_test>{
        // spell_damage_near_target spells - damage around target
        {"삼매진화",
         SPELL_TYPE::TARGET,
         [](const auto& bot) {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto mp_cost    = current_mp; // mp = me:mp() (uses all MP)
             return std::pair<int, int>{current_hp, current_mp - mp_cost};
         }                                        },

        {"지진",     SPELL_TYPE::TARGET, [](const auto& bot) {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto mp_cost    = 580; // Fixed MP cost
             return std::pair<int, int>{current_hp, current_mp - mp_cost};
         }}
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : near_target_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);
    fb::logger::info("Successfully learned {} out of {} near target damage spells",
                     learned_count,
                     near_target_spells.size());

    fb::logger::info("Learning {} near target damage spells", near_target_spells.size());
    auto spell_slot = 1;

    for (const auto& spell : near_target_spells)
    {
        fb::logger::info("Testing near target spell: {}", spell.name);

        // Spawn a target monster in front of the caster
        auto target = co_await caster->spawn_monster_relative_by_look("다람쥐", 0, 2, 32793, DEFAULT_TIMEOUT);

        // Spawn monsters around the target position (relative to target)
        std::ignore = co_await caster->spawn_monsters_relative_by_look("다람쥐",
                                                                       {
                                                                           {0,  1},
                                                                           {-1, 2},
                                                                           {1,  2},
                                                                           {0,  3}
        },
                                                                       32793,
                                                                       DEFAULT_TIMEOUT);

        // Set caster's current HP/MP for testing
        std::ignore = co_await caster->set_current_hp_mp(1000, 1000, DEFAULT_TIMEOUT);

        // Calculate expected values using the spell calculator function
        auto [expected_hp, expected_mp] = spell.calculator(caster);

        // Cast the spell targeting the monster in front
        std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", target.oid, target.position),
            [=](auto& resp) -> bool {
                return resp.ch_hp == expected_hp && resp.ch_mp == expected_mp;
            });

        spell_slot++;
    }

    caster->chat("=== NEAR TARGET DAMAGE SPELL TEST COMPLETED ===");
    fb::logger::info("Near target damage spell test completed.");
    co_return true;
}
