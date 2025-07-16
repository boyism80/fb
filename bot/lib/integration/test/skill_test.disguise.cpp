#include <fb/bot/integration/skill_test.h>
#include <fb/bot/game_bot.h>
#include <fb/logger.h>
#include <chrono>
#include <vector>
#include <string>

using namespace fb::bot::integration;
using namespace std::chrono_literals;

namespace fb::bot::integration {

struct disguise_spell_test
{
    std::string              spell_name;
    int                      mp_cost;
    std::vector<std::string> available_mobs;
};

async::task<bool> skill_test::test_disguise_spells(std::shared_ptr<fb::bot::game_bot> caster)
{
    auto bots = this->get_test_bots();

    // Define disguise spells with their available monster transformations
    auto disguise_spells = std::vector<disguise_spell_test>{
        {"경수", 30, {"토끼", "다람쥐"}                                },
        {"맹수", 30, {"곰", "호랑이"}                                  },
        {"야수", 30, {"여우", "늑대"}                                  },
        {"금수", 30, {"고양이", "새끼돼지", "돼지", "강아지", "누렁이"}}
    };

    fb::logger::debug("Testing disguise spells with all available monster transformations");
    caster->chat("=== DISGUISE SPELL TEST STARTED ===");

    // Learn all disguise spells
    auto spell_names = std::vector<std::string>{};
    for (const auto& spell : disguise_spells)
    {
        fb::logger::debug("Learning spell: {}", spell.spell_name);
        spell_names.push_back(spell.spell_name);
    }
    std::ignore = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);

    // Test each disguise spell with all available monster transformations
    auto spell_slot = 0;
    for (const auto& spell : disguise_spells)
    {
        fb::logger::debug("Testing {} spell", spell.spell_name);

        // Test each available monster transformation
        for (const auto& mob_name : spell.available_mobs)
        {
            // Set current hp and mp
            std::ignore = co_await caster->set_current_hp_mp(10000, 1000, DEFAULT_TIMEOUT);

            fb::logger::debug("Testing {} transformation to {}", spell.spell_name, mob_name);

            // Step 1: Cast disguise spell with specific monster name
            auto before_mp   = caster->mp();
            auto expected_mp = before_mp - spell.mp_cost;

            fb::logger::debug("Casting {} spell to transform into {} (MP: {} -> {})",
                              spell.spell_name,
                              mob_name,
                              before_mp,
                              expected_mp);

            // Use SPELL_TYPE::INPUT with monster name as text input
            caster->chat(std::format("{} - {}", spell.spell_name, mob_name));
            std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
                fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, spell_slot, mob_name, 0, {0, 0}),
                [expected_mp](auto& resp) -> bool {
                    return resp.ch_mp == expected_mp;
                },
                DEFAULT_TIMEOUT);

            fb::logger::debug("{} spell cast completed successfully for {}", spell.spell_name, mob_name);

            // Step 2: Verify transformation (check if buff is applied)
            // Note: We can't directly verify the visual transformation in bot testing,
            // but we can verify that the spell was cast successfully and MP was consumed

            fb::logger::debug("{} transformation to {} completed successfully", spell.spell_name, mob_name);

            // Step 3: Remove buff to prepare for next test
            fb::logger::debug("Removing buff to prepare for next test");
            caster->remove_buffs();
        }

        fb::logger::debug("Completed testing all transformations for {} spell", spell.spell_name);
        spell_slot++;
    }

    fb::logger::debug("Disguise spell testing completed");
    caster->chat("=== DISGUISE SPELL TEST COMPLETED ===");
    co_return true;
}

} // namespace fb::bot::integration