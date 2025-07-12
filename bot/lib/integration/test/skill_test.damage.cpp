#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_damage_spells(std::shared_ptr<fb::bot::game_bot> caster)
{
    fb::logger::info("Bot {} starting damage spell test", caster->oid());
    caster->chat("=== DAMAGE SPELL TEST STARTED ===");

    // Setup all bots with max HP/MP
    std::ignore = co_await caster->setup_bot_stats(100000, 100000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);

    std::vector<damage_spell_test> damage_spells = {
        {"뢰진주",       SPELL_TYPE::TARGET, 320,  120},
        {"화염주",       SPELL_TYPE::TARGET, 320,  120},
        {"백열주",       SPELL_TYPE::TARGET, 320,  120},
        {"자무주",       SPELL_TYPE::TARGET, 320,  120},
        {"뢰격주",       SPELL_TYPE::TARGET, 530,  180},
        {"화영열주",     SPELL_TYPE::TARGET, 530,  180},
        {"백령주",       SPELL_TYPE::TARGET, 530,  180},
        {"자영무주",     SPELL_TYPE::TARGET, 530,  180},
        {"뢰격참주",     SPELL_TYPE::TARGET, 740,  250},
        {"화열참주",     SPELL_TYPE::TARGET, 740,  250},
        {"백열참주",     SPELL_TYPE::TARGET, 740,  250},
        {"자천무주",     SPELL_TYPE::TARGET, 740,  250},
        {"진뢰격참주",   SPELL_TYPE::TARGET, 1950, 330},
        {"진화열참주",   SPELL_TYPE::TARGET, 1950, 330},
        {"진백열참주",   SPELL_TYPE::TARGET, 1950, 330},
        {"진자천무주",   SPELL_TYPE::TARGET, 1950, 330},
        {"극진뢰격참주", SPELL_TYPE::TARGET, 3580, 360},
        {"극진화열참주", SPELL_TYPE::TARGET, 3580, 360},
        {"극진백열참주", SPELL_TYPE::TARGET, 3580, 360},
        {"극진자천무주", SPELL_TYPE::TARGET, 3580, 360},
        {"흡성대법",     SPELL_TYPE::TARGET, 3000, 60 },
        {"백열장",       SPELL_TYPE::TARGET, 280,  80 },
        {"헬파이어",     SPELL_TYPE::TARGET, 0,    0  }  // Special case, damage is not fixed, mp cost is all current mp
    };
    auto spell_count = damage_spells.size();

    std::vector<std::string> spell_names;
    for (const auto& spell : damage_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);
    fb::logger::info("Successfully learned {} out of {} damage spells", learned_count, damage_spells.size());

    fb::logger::info("Learning {} damage spells", damage_spells.size());
    auto spell_slot = 1;
    for (const auto& spell : damage_spells)
    {
        fb::logger::info("Testing spell: {} (Damage: {}, MP: -{})",
                         spell.name,
                         spell.expected_damage,
                         spell.expected_mp_cost);

        auto mob_info = co_await caster->spawn_monster_relative_by_look("다람쥐", 0, 1, 32793, DEFAULT_TIMEOUT);

        // Set caster's current HP/MP for testing
        std::ignore = co_await caster->set_current_hp_mp(50, 100000, DEFAULT_TIMEOUT);

        auto before_caster_hp = caster->hp();
        auto before_caster_mp = caster->mp();
        auto expected_hp      = before_caster_hp;
        auto expected_mp      = before_caster_mp - spell.expected_mp_cost;
        if (spell.name == "헬파이어")
            expected_mp = 0;

        if (spell.name == "흡성대법")
            expected_hp = before_caster_hp + 15;

        caster->chat(std::format("Testing {}", spell.name));
        std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", mob_info.oid, mob_info.position),
            [=](auto& resp) -> bool {
                return resp.ch_hp == expected_hp && resp.ch_mp == expected_mp;
            },
            DEFAULT_TIMEOUT);
        spell_slot++;
    }

    caster->chat("=== DAMAGE SPELL TEST COMPLETED ===");
    fb::logger::info("Damage spell test completed.");
    co_return true;
}
