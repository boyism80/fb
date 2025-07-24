#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_healing_spells(std::shared_ptr<fb::bot::game_bot> caster)
{
    fb::logger::debug("Bot {} starting healing spell test", caster->oid());
    caster->chat("=== HEALING SPELL TEST STARTED ===");

    // Step 1: Setup bots with HP/MP for spell testing
    std::ignore = co_await caster->setup_bot_stats(100000, 100000, 50, std::nullopt, DEFAULT_TIMEOUT);
    std::ignore = co_await caster->setup_bot_stats(100000, 100000, 50, std::nullopt, DEFAULT_TIMEOUT);

    caster->chat("Bot setup completed - ready for spell testing");

    std::vector<healing_spell_test> healing_spells = {
        {"누리의기원", SPELL_TYPE::NORMAL, 50,    30   },
        {"하늘의기원", SPELL_TYPE::NORMAL, 200,   120  },
        {"대지의기원", SPELL_TYPE::TARGET, 50,    50   },
        {"동해의기원", SPELL_TYPE::TARGET, 100,   30   },
        {"바다의기원", SPELL_TYPE::NORMAL, 100,   50   },
        {"천공의기원", SPELL_TYPE::TARGET, 200,   100  },
        {"구름의기원", SPELL_TYPE::TARGET, 500,   120  },
        {"태양의기원", SPELL_TYPE::TARGET, 1000,  240  },
        {"생명의기원", SPELL_TYPE::TARGET, 5000,  300  },
        {"현자의기원", SPELL_TYPE::TARGET, 5000,  1000 },
        {"신령의기원", SPELL_TYPE::TARGET, 10000, 1000 },
        {"봉황의기원", SPELL_TYPE::TARGET, 30000, 10000},
        {"천공의희원", SPELL_TYPE::TARGET, 200,   200  },
        {"구름의희원", SPELL_TYPE::TARGET, 500,   480  },
        {"태양의희원", SPELL_TYPE::TARGET, 1000,  960  }
    };

    // Step 2: Learn all healing spells
    fb::logger::debug("Learning {} healing spells", healing_spells.size());

    std::vector<std::string> spell_names;
    for (const auto& spell : healing_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);
    fb::logger::debug("Successfully learned {} out of {} healing spells", learned_count, healing_spells.size());
    caster->chat("All healing spells learned - starting test sequence");

    co_await caster->change_base_hp(100000, DEFAULT_TIMEOUT);
    co_await caster->change_hp(50, DEFAULT_TIMEOUT);

    // Step 3: Test each healing spell
    auto spell_slot = 0;
    for (const auto& spell : healing_spells)
    {
        fb::logger::debug("Testing spell: {} (HP: +{}, MP: -{})",
                          spell.name,
                          spell.expected_hp_gain,
                          spell.expected_mp_cost);

        if (spell.type == SPELL_TYPE::NORMAL)
        {
            // Test self-healing spell
            auto before_hp = caster->hp();
            auto before_mp = caster->mp();

            fb::logger::debug("Casting {} on self - before: hp={}, mp={}", spell.name, before_hp, before_mp);

            caster->chat(std::format("Testing {}", spell.name));
            std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
                fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", 0, {0, 0}),
                [before_hp, spell](auto& resp) -> bool {
                    return resp.ch_hp == before_hp + spell.expected_hp_gain;
                },
                DEFAULT_TIMEOUT);

            fb::logger::debug("Successfully cast {} - after: hp={}, mp={}", spell.name, caster->hp(), caster->mp());
        }
        else
        {
            // Test caster healing spell
            auto before_caster_hp = caster->hp();
            auto before_caster_mp = caster->mp();

            fb::logger::debug("Casting {} on caster - caster hp={}, caster mp={}",
                              spell.name,
                              before_caster_hp,
                              before_caster_mp);

            // Caster casts spell on caster
            caster->chat(std::format("Testing {}", spell.name));
            std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
                fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", caster->oid(), caster->position()),
                [before_caster_mp, before_caster_hp, spell](auto& resp) -> bool {
                    if (resp.ch_mp != before_caster_mp - spell.expected_mp_cost)
                        return false;

                    if (resp.ch_hp != before_caster_hp + spell.expected_hp_gain)
                        return false;

                    return true;
                },
                DEFAULT_TIMEOUT);

            fb::logger::debug("Successfully cast {} on caster - caster hp={}, caster mp={}",
                              spell.name,
                              caster->hp(),
                              caster->mp());
        }

        spell_slot++;
    }

    // Step 4: Special test for dynamic spell
    fb::logger::debug("Testing special dynamic spell: 백호의희원");

    // Learn the dynamic spell
    auto index = co_await caster->learn_spell("백호의희원", DEFAULT_TIMEOUT);
    if (index == 0xFF)
    {
        fb::logger::warn("Failed to learn spell: 백호의희원");
        co_return false;
    }

    co_await caster->change_mp(100, DEFAULT_TIMEOUT);
    co_await caster->change_hp(50, DEFAULT_TIMEOUT);

    // Calculate based on current MP
    auto current_mp          = caster->mp();
    auto theoretical_hp_gain = current_mp * 2;
    auto expected_mp_cost    = current_mp / 2;

    // Check caster's max HP (set to 100000)
    auto caster_max_hp         = caster->base_hp();
    auto caster_current_hp     = caster->hp();
    auto caster_recoverable_hp = caster_max_hp - caster_current_hp;

    // Calculate actual recoverable amount (cannot exceed max HP)
    auto actual_expected_hp_gain = std::min(theoretical_hp_gain, caster_recoverable_hp);

    fb::logger::debug(
        "Dynamic spell calculation: current MP={}, theoretical HP gain={}, actual recoverable={}, expected MP cost={}",
        current_mp,
        theoretical_hp_gain,
        actual_expected_hp_gain,
        expected_mp_cost);

    // If theoretical gain exceeds recoverable amount, adjust caster HP for accurate testing
    if (theoretical_hp_gain > caster_recoverable_hp)
    {
        // Set caster's current HP lower to enable accurate test
        auto new_caster_hp = caster_max_hp - theoretical_hp_gain;
        if (new_caster_hp < 1)
            new_caster_hp = 1; // Maintain minimum 1 HP

        fb::logger::debug("Adjusting caster HP to {} for accurate dynamic spell test", new_caster_hp);

        co_await caster->change_hp(new_caster_hp, DEFAULT_TIMEOUT);

        // Recalculate actual recovery amount
        actual_expected_hp_gain = theoretical_hp_gain;
    }

    // Execute test
    auto before_caster_hp = caster->hp();
    auto before_caster_mp = caster->mp();

    fb::logger::debug("Casting dynamic spell on caster - caster hp={}, caster mp={}",
                      before_caster_hp,
                      before_caster_mp);

    // Cast the dynamic spell
    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, spell_slot, "", caster->oid(), caster->position()),
        [before_caster_mp, before_caster_hp, actual_expected_hp_gain, expected_mp_cost](auto& resp) -> bool {
            if (resp.ch_mp != before_caster_mp - expected_mp_cost)
                return false;

            if (resp.ch_hp != before_caster_hp + actual_expected_hp_gain)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Successfully cast dynamic spell on caster - caster hp={}, caster mp={}",
                      caster->hp(),
                      caster->mp());

    fb::logger::debug("Healing spell test completed successfully - {} healing spells + 1 special spell tested",
                      healing_spells.size());
    caster->chat("All healing spell tests completed successfully!");

    co_return true;
}
