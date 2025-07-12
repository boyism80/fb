#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_multi_target_attack_cast_spells(std::shared_ptr<fb::bot::game_bot> caster)
{
    fb::logger::info("Bot {} starting multi-target attack_cast spell test", caster->oid());
    caster->chat("=== MULTI-TARGET ATTACK_CAST SPELL TEST STARTED ===");

    // Setup all bots with max HP/MP
    std::ignore = co_await caster->setup_bot_stats(100000, 100000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);

    auto multi_target_spells = std::vector<multi_target_attack_cast_spell_test>{
        // Multi-target attack spells
        {"어검술",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot)
             -> std::tuple<int, int, std::optional<fb::model::point<uint16_t>>> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp * 2 / 3; // hp = me:hp()*2 // 3
             return {current_hp - hp_cost, current_mp - 40, std::optional<fb::model::point<uint16_t>>{}};
         }, {{0, 1}, {-1, 2}, {0, 2}, {1, 2}},                                           // 4 positions in front
         false                                          },

        {"극'백호참",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot)
             -> std::tuple<int, int, std::optional<fb::model::point<uint16_t>>> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2; // hp = me:hp()//2
             return {current_hp - hp_cost, current_mp - 50, std::optional<fb::model::point<uint16_t>>{}};
         }, {{0, 1}, {-1, 2}, {0, 2}, {1, 2}, {-2, 3}, {-1, 3}, {0, 3}, {1, 3}, {2, 3}}, // 9 positions in front
         false},

        {"파천검무",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot)
             -> std::tuple<int, int, std::optional<fb::model::point<uint16_t>>> {
             auto current_hp  = bot->hp();
             auto current_mp  = bot->mp();
             auto hp_cost     = current_hp / 2; // hp = me:hp()//2
             auto current_pos = bot->position();
             auto expected_pos =
                 fb::model::point<uint16_t>(current_pos.x, current_pos.y + uint16_t{5}); // Move 5 tiles forward
             return {current_hp - hp_cost, current_mp - 180, expected_pos};
         }, {{0, 1}, {0, 2}, {0, 3}, {0, 4}},                                            // 4 positions in a line
         true                                            }, // This spell moves the caster forward

        {"초혼비무",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot)
             -> std::tuple<int, int, std::optional<fb::model::point<uint16_t>>> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp * 2 / 3; // hp = me:hp()*2 // 3
             return {current_hp - hp_cost, current_mp - 140, std::optional<fb::model::point<uint16_t>>{}};
         }, {{0, 4}},                                                                    // 1 position at 4 tiles away
         false                                                                   }, // This spell moves the target, not the caster

        {"쇄혼비무",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot)
             -> std::tuple<int, int, std::optional<fb::model::point<uint16_t>>> {
             auto current_hp  = bot->hp();
             auto current_mp  = bot->mp();
             auto hp_cost     = current_hp * 2 / 3; // hp = me:hp()*2 // 3
             auto current_pos = bot->position();
             auto expected_pos =
                 fb::model::point<uint16_t>(current_pos.x, current_pos.y + uint16_t{3}); // Move 3 tiles forward
             return {current_hp - hp_cost, current_mp - 140, expected_pos};
         }, {{0, 4}},                                                                    // 1 position at 4 tiles away
         true                                                                    }  // This spell moves the caster 3 tiles forward
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : multi_target_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);
    fb::logger::info("Successfully learned {} out of {} multi-target attack_cast spells",
                     learned_count,
                     multi_target_spells.size());

    fb::logger::info("Learning {} multi-target attack_cast spells", multi_target_spells.size());
    auto spell_slot = 1;

    for (const auto& spell : multi_target_spells)
    {
        fb::logger::info("Testing multi-target spell: {}", spell.name);

        auto caster_pos = caster->position();

        // Spawn monsters at the calculated positions
        std::ignore =
            co_await caster->spawn_monsters_relative_by_look("다람쥐", spell.spawn_positions, 32793, DEFAULT_TIMEOUT);

        // Set caster's current HP/MP for testing
        std::ignore = co_await caster->set_current_hp_mp(1000, 1000, DEFAULT_TIMEOUT);

        // Calculate expected values using the spell calculator function
        auto [expected_hp, expected_mp, expected_position] = spell.calculator(caster);

        // Cast the spell
        std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", 0, {0, 0}),
            [=](auto& resp) -> bool {
                auto success = resp.ch_hp == expected_hp && resp.ch_mp == expected_mp;
                // Check position if expected_position is provided
                if (expected_position.has_value())
                {
                    auto current_pos = caster->position();
                    success          = success && (current_pos == expected_position.value());
                }
                return success;
            },
            DEFAULT_TIMEOUT);

        spell_slot++;

        // Move bot back to original position if it moved
        co_await caster->move_bot_back_to_position(caster_pos, DEFAULT_INTERVAL, DEFAULT_TIMEOUT);
    }

    caster->chat("=== MULTI-TARGET ATTACK_CAST SPELL TEST COMPLETED ===");
    fb::logger::info("Multi-target attack_cast spell test completed.");
    co_return true;
}
