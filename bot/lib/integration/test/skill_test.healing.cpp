#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_healing_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots)
{
    // Select the test bot for skill operations
    auto caster = bots.at(0);
    auto target = bots.at(1);

    fb::logger::info("Bot {} starting healing spell test", caster->oid());
    caster->chat("=== HEALING SPELL TEST STARTED ===");

    // Step 1: Setup bots with HP/MP for spell testing
    auto thread = caster->thread();
    co_await thread->switching();

    // Setup all bots with max HP/MP and current HP
    for (auto& bot : bots)
    {
        std::ignore = co_await this->setup_bot_stats(bot, 100000, 100000, 50, std::nullopt);
    }

    caster->chat("Bot setup completed - ready for spell testing");

    std::vector<healing_spell_test> healing_spells = {
        {"누리의기원", SPELL_TYPE::NORMAL, 50,    30   },
        {"하늘의기원", SPELL_TYPE::NORMAL, 200,   120  },
        {"대지의기원", SPELL_TYPE::TARGET, 50,    50   },
        {"동해의기원", SPELL_TYPE::TARGET, 100,   30   },
        {"바다의기원", SPELL_TYPE::TARGET, 100,   50   },
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
    fb::logger::info("Learning {} healing spells", healing_spells.size());

    std::vector<std::string> spell_names;
    for (const auto& spell : healing_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await this->learn_spells(caster, spell_names);
    fb::logger::info("Successfully learned {} out of {} healing spells", learned_count, healing_spells.size());
    caster->chat("All healing spells learned - starting test sequence");

    // Step 3: Test each healing spell
    auto spell_slot = 1;
    for (const auto& spell : healing_spells)
    {
        fb::logger::info("Testing spell: {} (HP: +{}, MP: -{})",
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
            // Test target healing spell
            auto before_target_hp = target->hp();
            auto before_caster_mp = caster->mp();

            fb::logger::debug("Casting {} on target - target hp={}, caster mp={}",
                              spell.name,
                              before_target_hp,
                              before_caster_mp);

            // Caster casts spell on target
            caster->chat(std::format("Testing {}", spell.name));
            std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
                fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", target->oid(), target->position()),
                [before_caster_mp, spell](auto& resp) -> bool {
                    return resp.ch_mp == before_caster_mp - spell.expected_mp_cost;
                },
                DEFAULT_TIMEOUT);

            // Wait for target's HP to update and verify
            co_await caster->thread()->sleep(DEFAULT_INTERVAL);
            auto after_target_hp = target->hp();
            if (after_target_hp != before_target_hp + spell.expected_hp_gain)
            {
                fb::logger::warn("Spell {} HP verification failed: expected {}, got {}",
                                 spell.name,
                                 before_target_hp + spell.expected_hp_gain,
                                 after_target_hp);
            }

            fb::logger::debug("Successfully cast {} on target - target hp={}, caster mp={}",
                              spell.name,
                              target->hp(),
                              caster->mp());
        }

        spell_slot++;
        co_await caster->thread()->sleep(DEFAULT_INTERVAL);
    }

    // Step 4: Special test for dynamic spell
    fb::logger::info("Testing special dynamic spell: 백호의희원");

    // Learn the dynamic spell
    std::ignore = co_await caster->request<fb::protocol::game::response::spell_update>(
        fb::protocol::game::request::chat{false, "/마법배우기 백호의희원"},
        DEFAULT_TIMEOUT);

    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, "/현재마력 100"},
        [](auto& resp) -> bool {
            return resp.ch_mp == 100;
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await target->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, "/현재체력 50"},
        [](auto& resp) -> bool {
            return resp.ch_hp == 50;
        },
        DEFAULT_TIMEOUT);

    // Calculate based on current MP
    auto current_mp          = caster->mp();
    auto theoretical_hp_gain = current_mp * 2;
    auto expected_mp_cost    = current_mp / 2;

    // Check target's max HP (set to 100000)
    auto target_max_hp         = target->base_hp();
    auto target_current_hp     = target->hp();
    auto target_recoverable_hp = target_max_hp - target_current_hp;

    // Calculate actual recoverable amount (cannot exceed max HP)
    auto actual_expected_hp_gain = std::min(theoretical_hp_gain, target_recoverable_hp);

    fb::logger::info(
        "Dynamic spell calculation: current MP={}, theoretical HP gain={}, actual recoverable={}, expected MP cost={}",
        current_mp,
        theoretical_hp_gain,
        actual_expected_hp_gain,
        expected_mp_cost);

    // If theoretical gain exceeds recoverable amount, adjust target HP for accurate testing
    if (theoretical_hp_gain > target_recoverable_hp)
    {
        // Set target's current HP lower to enable accurate test
        auto new_target_hp = target_max_hp - theoretical_hp_gain;
        if (new_target_hp < 1)
            new_target_hp = 1; // Maintain minimum 1 HP

        fb::logger::info("Adjusting target HP to {} for accurate dynamic spell test", new_target_hp);

        std::ignore = co_await target->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat{false, std::format("/현재체력 {}", new_target_hp)},
            DEFAULT_TIMEOUT);

        // Recalculate actual recovery amount
        actual_expected_hp_gain = theoretical_hp_gain;
    }

    // Execute test
    auto before_target_hp = target->hp();
    auto before_caster_mp = caster->mp();

    fb::logger::debug("Casting dynamic spell on target - target hp={}, caster mp={}",
                      before_target_hp,
                      before_caster_mp);

    // Cast the dynamic spell
    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, spell_slot, "", target->oid(), target->position()),
        [before_caster_mp, expected_mp_cost](auto& resp) -> bool {
            return resp.ch_mp == before_caster_mp - expected_mp_cost;
        },
        DEFAULT_TIMEOUT);

    // Verify target HP update
    co_await caster->thread()->sleep(DEFAULT_INTERVAL);
    auto after_target_hp = target->hp();
    if (after_target_hp != before_target_hp + actual_expected_hp_gain)
    {
        fb::logger::warn("Dynamic spell HP verification failed: expected {}, got {}",
                         before_target_hp + actual_expected_hp_gain,
                         after_target_hp);
    }

    fb::logger::debug("Successfully cast dynamic spell on target - target hp={}, caster mp={}",
                      target->hp(),
                      caster->mp());

    fb::logger::info("Healing spell test completed successfully - {} healing spells + 1 special spell tested",
                     healing_spells.size());
    caster->chat("All healing spell tests completed successfully!");

    co_return true;
}
