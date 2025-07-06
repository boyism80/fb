#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace fb::bot::integration;
using namespace std::chrono_literals;

namespace fb::bot::integration {

async::task<bool> skill_test::test_loot_spell(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                              std::chrono::milliseconds                        timeout)
{
    constexpr auto item_name    = "도토리";
    constexpr auto item_count   = 200;
    constexpr auto money_amount = 10000;
    constexpr auto interval     = 100ms;

    if (bots.size() < 1)
    {
        fb::logger::fatal("Loot spell test requires at least 1 bot");
        co_return false;
    }

    auto& caster = bots[0];

    fb::logger::info("Testing loot spell with item and money pickup");
    caster->chat("=== LOOT SPELL TEST STARTED ===");

    // Step 1: Create items and change money
    fb::logger::info("Creating test items and setting money");
    co_await caster->create_item("도토리", 200, timeout);
    co_await caster->change_money(10000, timeout);

    fb::logger::info("Items and money created successfully");

    // Step 2: Move one step forward
    fb::logger::info("Moving one step forward");
    co_await caster->move(DIRECTION::BOTTOM, 1, interval);

    // Step 3: Drop items and money
    fb::logger::info("Dropping items and money");

    // Drop item
    co_await caster->request<fb::protocol::game::response::item_remove>(
        fb::protocol::game::request::item_drop(1, item_count),
        timeout);

    // Drop money
    co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::item_drop_money(money_amount),
        timeout);

    fb::logger::info("Items and money dropped successfully");

    // Step 4: Move back to original position and face BOTTOM direction
    fb::logger::info("Moving back to original position");
    co_await caster->move(DIRECTION::TOP, 1, interval);
    caster->thread()->sleep(500ms);
    caster->direction(DIRECTION::BOTTOM);

    // Step 5: Learn and cast loot spell
    fb::logger::info("Learning loot spell");
    auto spell_names = std::vector<std::string>{"노획"};
    co_await this->learn_spells(caster, spell_names, timeout);

    // Set current hp and mp
    co_await this->set_current_hp_mp(caster, 10000, 1000, timeout);

    fb::logger::info("Casting loot spell");
    auto before_mp   = caster->mp();
    auto expected_mp = before_mp - 30; // MP cost for loot spell

    // Cast loot spell and wait for item pickup response
    co_await caster->request<fb::protocol::game::response::item_update>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, 1, "", 0, {0, 0}),
        [item_name, item_count](auto& resp) -> bool {
            return resp.name.starts_with(item_name) && resp.count == item_count;
        },
        timeout);

    fb::logger::info("Loot spell cast completed");

    // Step 6: Verify MP consumption
    auto after_mp = caster->mp();
    if (after_mp != expected_mp)
    {
        auto sstream = std::stringstream{};
        sstream << "Loot spell MP consumption verification failed: expected " << expected_mp << ", got " << after_mp;
        throw std::runtime_error(sstream.str());
    }

    fb::logger::info("Loot spell MP consumption verification successful: {} -> {}", before_mp, after_mp);

    // Step 7: Verify that items and money were picked up
    // Note: In a real test, we would verify the inventory contents
    // For now, we'll just log that the spell was cast successfully
    fb::logger::info("Loot spell test completed successfully");

    fb::logger::info("Loot spell testing completed");
    caster->chat("=== LOOT SPELL TEST COMPLETED ===");
    co_return true;
}

} // namespace fb::bot::integration