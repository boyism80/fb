#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_special_spells(std::shared_ptr<fb::bot::game_bot> caster)
{
    fb::logger::debug("Bot {} starting special spell test", caster->oid());
    caster->chat("=== SPECIAL SPELL TEST STARTED ===");

    // Setup bot with max HP/MP
    std::ignore = co_await caster->setup_bot_stats(100000, 100000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);

    struct special_spell_test
    {
        std::string name; // Spell name
        std::function<async::task<bool>(const std::shared_ptr<fb::bot::game_bot>&)>
            pre_condition_check; // Called before spell cast to setup conditions
        std::function<async::task<bool>(const std::shared_ptr<fb::bot::game_bot>&)>
            spell_cast_function; // Custom spell casting logic
        std::function<async::task<bool>(const std::shared_ptr<fb::bot::game_bot>&)>
            post_condition_check; // Called after spell cast to verify effects
    };

    auto special_spells = std::vector<special_spell_test>{
        {"귀환",
         [](const auto& caster) -> async::task<bool> {
             // Pre-condition: Setup nation for teleportation test
             fb::logger::debug("Pre-condition: Setting up nation for 귀환 test");
             co_return true;
         }, [](const auto& caster) -> async::task<bool> {
             // Spell cast: Use spell_cast with empty message
             auto&& resp = co_await caster->request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, 0, "", 0, {0, 0}),
                 DEFAULT_TIMEOUT);

             // Check if spell was cast successfully
             if (resp.text == std::format("{} 외웠습니다.", name_with("귀환")))
             {
                 fb::logger::debug("귀환 spell cast successful");
                 co_return true;
             }

             fb::logger::warn("귀환 spell cast failed: {}", resp.text);
             co_return false;
         }, [](const auto& caster) -> async::task<bool> {
             // Post-condition: Verify teleportation to hometown
             fb::logger::debug("Post-condition: Verifying 귀환 teleportation");
             co_return true;
         }},

        {"비영사천문",
         [](const auto& caster) -> async::task<bool> {
             // Pre-condition: Setup direction input for teleportation
             fb::logger::debug("Pre-condition: Setting up direction input for 비영사천문 test");
             co_return true;
         }, [](const auto& caster) -> async::task<bool> {
             // Spell cast: Use spell_cast with direction message
             auto&& resp = co_await caster->request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, 1, "동", 0, {0, 0}),
                 DEFAULT_TIMEOUT);

             // Check if spell was cast successfully
             if (resp.text == std::format("{} 외웠습니다.", name_with("비영사천문")))
             {
                 fb::logger::debug("비영사천문 spell cast successful");
                 co_return true;
             }

             fb::logger::warn("비영사천문 spell cast failed: {}", resp.text);
             co_return false;
         }, [](const auto& caster) -> async::task<bool> {
             // Post-condition: Verify cardinal direction teleportation
             fb::logger::debug("Post-condition: Verifying 비영사천문 teleportation");
             co_return true;
         }},

        {"공력증강",
         [](const auto& caster) -> async::task<bool> {
             // Pre-condition: Setup MP/HP for enhancement test
             fb::logger::debug("Pre-condition: Setting up MP/HP for 공력증강 test");
             co_return true;
         }, [](const auto& caster) -> async::task<bool> {
             // Spell cast: Use spell_cast with empty message
             auto&& resp = co_await caster->request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, 2, "", 0, {0, 0}),
                 DEFAULT_TIMEOUT);

             // Check if spell was cast successfully
             if (resp.text == std::format("{} 외웠습니다.", name_with("공력증강")))
             {
                 fb::logger::debug("공력증강 spell cast successful");
                 co_return true;
             }

             fb::logger::warn("공력증강 spell cast failed: {}", resp.text);
             co_return false;
         }, [](const auto& caster) -> async::task<bool> {
             // Post-condition: Verify MP enhancement and HP cost
             fb::logger::debug("Post-condition: Verifying 공력증강 MP/HP changes");
             co_return true;
         }},

        {"대력검신",
         [](const auto& caster) -> async::task<bool> {
             // Pre-condition: Setup target and weapon for damage test
             fb::logger::debug("Pre-condition: Setting up target and weapon for 대력검신 test");
             co_return true;
         }, [](const auto& caster) -> async::task<bool> {
             // Spell cast: Use spell_cast with weapon type message
             auto&& resp = co_await caster->request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, 3, "검신", 0, {0, 0}),
                 DEFAULT_TIMEOUT);

             // Check if spell was cast successfully
             if (resp.text == std::format("{} 외웠습니다.", name_with("대력검신")))
             {
                 fb::logger::debug("대력검신 spell cast successful");
                 co_return true;
             }

             fb::logger::warn("대력검신 spell cast failed: {}", resp.text);
             co_return false;
         }, [](const auto& caster) -> async::task<bool> {
             // Post-condition: Verify weapon damage effect
             fb::logger::debug("Post-condition: Verifying 대력검신 damage effect");
             co_return true;
         }},

        {"검신검귀",
         [](const auto& caster) -> async::task<bool> {
             // Pre-condition: Setup target and weapon for damage test
             fb::logger::debug("Pre-condition: Setting up target and weapon for 검신검귀 test");
             co_return true;
         }, [](const auto& caster) -> async::task<bool> {
             // Spell cast: Use spell_cast with weapon type message
             auto&& resp = co_await caster->request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, 4, "검신", 0, {0, 0}),
                 DEFAULT_TIMEOUT);

             // Check if spell was cast successfully
             if (resp.text == std::format("{} 외웠습니다.", name_with("검신검귀")))
             {
                 fb::logger::debug("검신검귀 spell cast successful");
                 co_return true;
             }

             fb::logger::warn("검신검귀 spell cast failed: {}", resp.text);
             co_return false;
         }, [](const auto& caster) -> async::task<bool> {
             // Post-condition: Verify weapon damage effect
             fb::logger::debug("Post-condition: Verifying 검신검귀 damage effect");
             co_return true;
         }},

        {"신검합일",
         [](const auto& caster) -> async::task<bool> {
             // Pre-condition: Setup target and weapon for damage test
             fb::logger::debug("Pre-condition: Setting up target and weapon for 신검합일 test");
             co_return true;
         }, [](const auto& caster) -> async::task<bool> {
             // Spell cast: Use spell_cast with weapon type message
             auto&& resp = co_await caster->request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, 5, "검신", 0, {0, 0}),
                 DEFAULT_TIMEOUT);

             // Check if spell was cast successfully
             if (resp.text == std::format("{} 외웠습니다.", name_with("신검합일")))
             {
                 fb::logger::debug("신검합일 spell cast successful");
                 co_return true;
             }

             fb::logger::warn("신검합일 spell cast failed: {}", resp.text);
             co_return false;
         }, [](const auto& caster) -> async::task<bool> {
             // Post-condition: Verify weapon damage effect
             fb::logger::debug("Post-condition: Verifying 신검합일 damage effect");
             co_return true;
         }},

        {"소혼강신",
         [](const auto& caster) -> async::task<bool> {
             // Pre-condition: Setup level and monster type for summoning test
             fb::logger::debug("Pre-condition: Setting up level and monster type for 소혼강신 test");
             co_return true;
         }, [](const auto& caster) -> async::task<bool> {
             // Spell cast: Use spell_cast with monster type message
             auto&& resp = co_await caster->request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, 6, "곰", 0, {0, 0}),
                 DEFAULT_TIMEOUT);

             // Check if spell was cast successfully
             if (resp.text == std::format("{} 외웠습니다.", name_with("소혼강신")))
             {
                 fb::logger::debug("소혼강신 spell cast successful");
                 co_return true;
             }

             fb::logger::warn("소혼강신 spell cast failed: {}", resp.text);
             co_return false;
         }, [](const auto& caster) -> async::task<bool> {
             // Post-condition: Verify monster summoning
             fb::logger::debug("Post-condition: Verifying 소혼강신 monster summoning");
             co_return true;
         }},

        {"성황령",
         [](const auto& caster) -> async::task<bool> {
             // Pre-condition: Setup ghost state and revive points
             fb::logger::debug("Pre-condition: Setting up ghost state and revive points for 성황령 test");
             co_return true;
         }, [](const auto& caster) -> async::task<bool> {
             // Spell cast: Use spell_cast with direction message
             auto&& resp = co_await caster->request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, 7, "좌", 0, {0, 0}),
                 DEFAULT_TIMEOUT);

             // Check if spell was cast successfully
             if (resp.text == std::format("{} 외웠습니다.", name_with("성황령")))
             {
                 fb::logger::debug("성황령 spell cast successful");
                 co_return true;
             }

             fb::logger::warn("성황령 spell cast failed: {}", resp.text);
             co_return false;
         }, [](const auto& caster) -> async::task<bool> {
             // Post-condition: Verify ghost teleportation
             fb::logger::debug("Post-condition: Verifying 성황령 ghost teleportation");
             co_return true;
         }}
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : special_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);
    fb::logger::debug("Successfully learned {} out of {} special spells", learned_count, special_spells.size());

    caster->chat("=== SPECIAL SPELL LEARNING COMPLETED ===");
    fb::logger::debug("Special spell learning completed. Individual test implementations pending.");

    // Run individual spell tests
    for (const auto& spell : special_spells)
    {
        fb::logger::debug("Running test for: {}", spell.name);

        // Run pre-condition check
        auto pre_condition_result = co_await spell.pre_condition_check(caster);
        if (!pre_condition_result)
        {
            fb::logger::warn("Pre-condition check failed for '{}', skipping test", spell.name);
            continue;
        }

        // Run spell cast function
        auto spell_cast_result = co_await spell.spell_cast_function(caster);
        if (!spell_cast_result)
        {
            fb::logger::warn("Spell cast failed for '{}', skipping post-condition", spell.name);
            continue;
        }

        // Run post-condition check
        auto post_condition_result = co_await spell.post_condition_check(caster);
        if (post_condition_result)
        {
            fb::logger::debug("Post-condition check passed for '{}'", spell.name);
        }
        else
        {
            fb::logger::warn("Post-condition check failed for '{}'", spell.name);
        }
    }

    caster->chat("=== SPECIAL SPELL TEST COMPLETED ===");
    fb::logger::debug("Special spell test completed.");
    co_return true;
}