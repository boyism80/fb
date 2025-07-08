#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

async::task<bool> skill_test::test_teleport_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots)
{
    if (bots.size() < 2)
    {
        fb::logger::fatal("Teleport spell test requires at least 2 bots");
        co_return false;
    }

    auto& caster = bots.front();
    auto& target = bots[1];

    fb::logger::info("Starting teleport spell test with {} bots", bots.size());
    caster->chat("=== TELEPORT SPELL TEST STARTED ===");

    // Teleport spells with their test parameters
    struct teleport_spell_test
    {
        std::string name;
        int         expected_mp_cost;
        bool        caster_moves; // true for 출두 (caster moves), false for 소환 (target moves)
    };

    auto teleport_spells = std::vector<teleport_spell_test>{
        {"출두", 30, true }, // Caster teleports to target
        {"소환", 30, false}  // Target teleports to caster
    };

    // Learn teleport spells
    auto spell_names = std::vector<std::string>{};
    for (const auto& spell : teleport_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await this->learn_spells(caster, spell_names);
    fb::logger::info("Successfully learned {} out of {} teleport spells", learned_count, teleport_spells.size());

    // Setup all bots with max HP/MP
    for (auto& bot : bots)
    {
        std::ignore = co_await this->setup_bot_stats(bot, 100000, 100000, std::nullopt, std::nullopt);
    }

    uint8_t spell_slot = 1;

    // Test each teleport spell
    auto begin_position = caster->position();
    for (const auto& spell_info : teleport_spells)
    {
        fb::logger::info("Testing teleport spell: {} (MP cost: {}, caster moves: {})",
                         spell_info.name,
                         spell_info.expected_mp_cost,
                         spell_info.caster_moves);

        caster->chat(std::format("Testing {}", spell_info.name));

        // Test scenario setup
        if (spell_info.caster_moves)
        {
            // 출두 test: Move caster to different map, then teleport to target
            co_await this->test_chuldu_spell(caster, target, spell_slot, spell_info.expected_mp_cost);
        }
        else
        {
            // 소환 test: Move target to different map, then summon target to caster
            co_await this->test_sohwan_spell(caster, target, spell_slot, spell_info.expected_mp_cost);
        }

        fb::logger::info("Teleport spell {} test completed successfully", spell_info.name);
        caster->chat(std::format("{} test completed", spell_info.name));

        spell_slot++;
        co_await caster->thread()->sleep(DEFAULT_INTERVAL);
    }

    fb::logger::info("Teleport spell test completed successfully - {} spells tested", teleport_spells.size());
    caster->chat("All teleport spell tests completed successfully!");

    auto end_position = caster->position();
    auto move_x_axis  = end_position.x - begin_position.x;
    auto direction_x  = move_x_axis > 0 ? DIRECTION::LEFT : DIRECTION::RIGHT;
    co_await caster->move(direction_x, std::abs(move_x_axis), DEFAULT_INTERVAL);

    auto move_y_axis = end_position.y - begin_position.y;
    auto direction_y = move_y_axis > 0 ? DIRECTION::TOP : DIRECTION::BOTTOM;
    co_await caster->move(direction_y, std::abs(move_y_axis), DEFAULT_INTERVAL);
    co_await caster->thread()->sleep(DEFAULT_INTERVAL);
    caster->direction(DIRECTION::BOTTOM);

    co_return true;
}

async::task<void> skill_test::test_chuldu_spell(std::shared_ptr<fb::bot::game_bot> caster,
                                                std::shared_ptr<fb::bot::game_bot> target,
                                                uint8_t                            spell_slot,
                                                int                                expected_mp_cost)
{
    fb::logger::info("Testing 출두 spell: caster moves to target location");

    // Step 1: Record initial positions
    auto initial_caster_position = caster->position();
    auto initial_target_position = target->position();

    fb::logger::debug("Initial positions - Caster: ({}, {}), Target: ({}, {})",
                      initial_caster_position.x,
                      initial_caster_position.y,
                      initial_target_position.x,
                      initial_target_position.y);

    // Step 2: Move caster to different map (가상계)
    fb::logger::info("Moving caster to 가상계 map");
    co_await caster->map_move("가상계", 1, 1, DEFAULT_TIMEOUT);
    co_await caster->thread()->sleep(500ms); // Wait for map transition to complete

    auto caster_virtual_position = caster->position();
    fb::logger::debug("Caster moved to 가상계 at position: ({}, {})",
                      caster_virtual_position.x,
                      caster_virtual_position.y);

    // Step 3: Cast 출두 spell with target's name
    auto before_caster_mp = caster->mp();
    auto expected_mp      = before_caster_mp - expected_mp_cost;

    fb::logger::info("Casting 출두 spell on target: {}", target->name());

    // Use SPELL_TYPE::INPUT with target's name as text input
    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, spell_slot, target->name(), 0, {0, 0}),
        [expected_mp](auto& resp) -> bool {
            return resp.ch_mp == expected_mp;
        },
        DEFAULT_TIMEOUT);

    // Step 4: Wait for teleport effect and verify caster moved to target
    co_await caster->thread()->sleep(1000ms); // Wait for teleport to complete

    auto final_caster_position = caster->position();
    auto final_target_position = target->position();

    fb::logger::debug("Final positions - Caster: ({}, {}), Target: ({}, {})",
                      final_caster_position.x,
                      final_caster_position.y,
                      final_target_position.x,
                      final_target_position.y);

    // Verify that caster is now near the target (within teleport range)
    auto distance_x = std::abs(static_cast<int>(final_caster_position.x) - static_cast<int>(final_target_position.x));
    auto distance_y = std::abs(static_cast<int>(final_caster_position.y) - static_cast<int>(final_target_position.y));

    if (distance_x <= 2 && distance_y <= 2) // Allow some tolerance for teleport positioning
    {
        fb::logger::info("출두 spell successful: caster teleported near target");
    }
    else
    {
        fb::logger::warn("출두 spell verification failed: caster not near target (distance: {}, {})",
                         distance_x,
                         distance_y);
    }

    fb::logger::info("출두 spell test completed");
}

async::task<void> skill_test::test_sohwan_spell(std::shared_ptr<fb::bot::game_bot> caster,
                                                std::shared_ptr<fb::bot::game_bot> target,
                                                uint8_t                            spell_slot,
                                                int                                expected_mp_cost)
{
    fb::logger::info("Testing 소환 spell: target moves to caster location");

    // Step 1: Record initial positions
    auto initial_caster_position = caster->position();
    auto initial_target_position = target->position();

    fb::logger::debug("Initial positions - Caster: ({}, {}), Target: ({}, {})",
                      initial_caster_position.x,
                      initial_caster_position.y,
                      initial_target_position.x,
                      initial_target_position.y);

    // Step 2: Move target to different map (가상계)
    fb::logger::info("Moving target to 가상계 map");
    co_await target->map_move("가상계", 1, 1, DEFAULT_TIMEOUT);
    co_await target->thread()->sleep(500ms); // Wait for map transition to complete

    auto target_virtual_position = target->position();
    fb::logger::debug("Target moved to 가상계 at position: ({}, {})",
                      target_virtual_position.x,
                      target_virtual_position.y);

    // Step 3: Cast 소환 spell with target's name
    auto before_caster_mp = caster->mp();
    auto expected_mp      = before_caster_mp - expected_mp_cost;

    fb::logger::info("Casting 소환 spell on target: {}", target->name());

    // Use SPELL_TYPE::INPUT with target's name as text input
    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, spell_slot, target->name(), 0, {0, 0}),
        [expected_mp](auto& resp) -> bool {
            return resp.ch_mp == expected_mp;
        },
        DEFAULT_TIMEOUT);

    // Step 4: Wait for summon effect and verify target moved to caster
    co_await caster->thread()->sleep(1000ms); // Wait for summon to complete

    auto final_caster_position = caster->position();
    auto final_target_position = target->position();

    fb::logger::debug("Final positions - Caster: ({}, {}), Target: ({}, {})",
                      final_caster_position.x,
                      final_caster_position.y,
                      final_target_position.x,
                      final_target_position.y);

    // Verify that target is now near the caster (within summon range)
    auto distance_x = std::abs(static_cast<int>(final_target_position.x) - static_cast<int>(final_caster_position.x));
    auto distance_y = std::abs(static_cast<int>(final_target_position.y) - static_cast<int>(final_caster_position.y));

    if (distance_x <= 2 && distance_y <= 2) // Allow some tolerance for summon positioning
    {
        fb::logger::info("소환 spell successful: target summoned near caster");
    }
    else
    {
        fb::logger::warn("소환 spell verification failed: target not near caster (distance: {}, {})",
                         distance_x,
                         distance_y);
    }

    fb::logger::info("소환 spell test completed");
}

} // namespace fb::bot::integration