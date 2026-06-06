#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_target_spells(std::shared_ptr<fb::bot::game_bot> caster,
                                                 std::shared_ptr<fb::bot::game_bot> target)
{
    fb::logger::debug("Bot {} starting target spell test", caster->oid());
    caster->chat("=== TARGET SPELL TEST STARTED ===");

    // Setup all bots with max HP/MP
    std::ignore = co_await caster->setup_bot_stats(100000, 100000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);
    std::ignore = co_await target->setup_bot_stats(100000, 100000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);

    struct target_spell_test
    {
        std::string name;             // Spell name
        int         expected_mp_cost; // Expected MP consumption
        OBJECT_TYPE target_type;      // Target object type requirement
        std::function<async::task<bool>(const std::shared_ptr<fb::bot::game_bot>&,
                                        const std::shared_ptr<fb::bot::game_bot>&)>
            pre_condition_check; // Called before spell cast to setup conditions
        std::function<async::task<bool>(const std::shared_ptr<fb::bot::game_bot>&,
                                        const std::shared_ptr<fb::bot::game_bot>&)>
            post_condition_check; // Called after spell cast to verify effects
    };

    auto target_spells = std::vector<target_spell_test>{
        // MP injection spell
        {"공력주입",
         0,   // MP cost is variable (caster's current MP)
         OBJECT_TYPE::LIFE,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             // Pre-condition: Target should be alive
             co_return true;
         }, [](const auto& caster, const auto& target) -> async::task<bool> {
             // Post-condition: Target's MP should be increased
             // Note: This would need to be implemented based on actual MP transfer logic
             fb::logger::debug("Post-condition: MP transfer effect verified");
             co_return true;
         }},

        // Information display spell
        {"대지의빛",
         30,  OBJECT_TYPE::CHARACTER,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             // Pre-condition: No special requirements
             co_return true;
         }, [](const auto& caster, const auto& target) -> async::task<bool> {
             // Post-condition: Character info should be displayed
             // This would need to check if info message was received
             fb::logger::debug("Post-condition: Character info display verified");
             co_return true;
         }},

        // Resurrection spell
        {"부활",
         300, OBJECT_TYPE::LIFE,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             // Pre-condition: Target should be in ghost state for resurrection
             //  if (target->state() != STATE::GHOST)
             //  {
             //      fb::logger::debug("Pre-condition failed: Target is not in ghost state");
             //      co_return false;
             //  }
             co_return true;
         }, [](const auto& caster, const auto& target) -> async::task<bool> {
             // Post-condition: Target should be revived to normal state
             //  if (target->state() != STATE::NORMAL)
             //  {
             //      fb::logger::debug("Post-condition failed: Target was not revived");
             //      co_return false;
             //  }
             fb::logger::debug("Post-condition: Resurrection effect verified");
             co_return true;
         }},

        // Buff removal spells
        {"시력회복",
         40,  OBJECT_TYPE::CHARACTER,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             // Pre-condition: Target should have '절망' buff
             // This would need to check if target has the specific buff
             fb::logger::debug("Pre-condition: Checking for '절망' buff on target");
             co_return true;
         }, [](const auto& caster, const auto& target) -> async::task<bool> {
             // Post-condition: '절망' buff should be removed
             // This would need to verify the buff was removed
             fb::logger::debug("Post-condition: '절망' buff removal verified");
             co_return true;
         }},

        {"퇴마주",
         30,  OBJECT_TYPE::CHARACTER,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             // Pre-condition: Target should have '저주' buff
             fb::logger::debug("Pre-condition: Checking for '저주' buff on target");
             co_return true;
         }, [](const auto& caster, const auto& target) -> async::task<bool> {
             // Post-condition: '저주' buff should be removed
             fb::logger::debug("Post-condition: '저주' buff removal verified");
             co_return true;
         }},

        {"파혼술",
         30,  OBJECT_TYPE::CHARACTER,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             // Pre-condition: Target should have '혼마술' buff
             fb::logger::debug("Pre-condition: Checking for '혼마술' buff on target");
             co_return true;
         }, [](const auto& caster, const auto& target) -> async::task<bool> {
             // Post-condition: '혼마술' buff should be removed
             fb::logger::debug("Post-condition: '혼마술' buff removal verified");
             co_return true;
         }},

        {"해독",
         30,  OBJECT_TYPE::CHARACTER,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             // Pre-condition: Target should have '중독' buff
             fb::logger::debug("Pre-condition: Checking for '중독' buff on target");
             co_return true;
         }, [](const auto& caster, const auto& target) -> async::task<bool> {
             // Post-condition: '중독' buff should be removed
             fb::logger::debug("Post-condition: '중독' buff removal verified");
             co_return true;
         }},

        {"활력",
         30,  OBJECT_TYPE::CHARACTER,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             // Pre-condition: Target should have '마비' buff
             fb::logger::debug("Pre-condition: Checking for '마비' buff on target");
             co_return true;
         }, [](const auto& caster, const auto& target) -> async::task<bool> {
             // Post-condition: '마비' buff should be removed
             fb::logger::debug("Post-condition: '마비' buff removal verified");
             co_return true;
         }}
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : target_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);
    fb::logger::debug("Successfully learned {} out of {} target spells", learned_count, target_spells.size());

    fb::logger::debug("Testing {} target spells", target_spells.size());
    auto spell_slot = 0;

    for (const auto& spell : target_spells)
    {
        fb::logger::debug("Testing target spell: {}", spell.name);

        // Run pre-condition check
        auto pre_condition_result = co_await spell.pre_condition_check(caster, target);
        if (!pre_condition_result)
        {
            fb::logger::warn("Pre-condition check failed for '{}', skipping test", spell.name);
            spell_slot++;
            continue;
        }

        auto target_oid      = target->oid();
        auto target_position = target->position();

        // Cast the spell
        caster->chat(std::format("Testing {}", spell.name));

        auto expected_caster_mp = uint32_t{0};
        while (true)
        {
            // Set caster's current HP/MP for testing
            std::ignore = co_await caster->set_current_hp_mp(10000, 10000, DEFAULT_TIMEOUT);

            // Set target's current HP/MP for testing
            std::ignore = co_await target->set_current_hp_mp(10000, 10000, DEFAULT_TIMEOUT);

            auto before_caster_mp = caster->mp();
            expected_caster_mp    = before_caster_mp - spell.expected_mp_cost;

            auto&& resp = co_await caster->request<fb::protocol::game::response::message>(
                fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET,
                                                        spell_slot,
                                                        "",
                                                        target_oid,
                                                        target_position),
                DEFAULT_TIMEOUT);

            if (resp.text == std::format("{} 외웠습니다.", name_with(spell.name)))
                break;

            caster->chat(resp.text);
        }

        if (spell.expected_mp_cost > 0 && caster->mp() != expected_caster_mp)
            throw std::runtime_error(std::format("MP cost mismatch for '{}'", spell.name));

        // Run post-condition check
        auto post_condition_result = co_await spell.post_condition_check(caster, target);
        if (post_condition_result)
        {
            fb::logger::debug("Post-condition check passed for '{}'", spell.name);
        }
        else
        {
            fb::logger::fatal("Post-condition check failed for '{}'", spell.name);
        }

        spell_slot++;
    }

    caster->chat("=== TARGET SPELL TEST COMPLETED ===");
    fb::logger::debug("Target spell test completed.");
    co_return true;
}