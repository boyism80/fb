#include <fb/bot/integration/swap_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

swap_test::swap_test(game_bot_controller& controller) :
    bot_integration_test(controller, 2) // Spawn 2 bots for testing
{ }

async::task<void> swap_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);

    auto bots = this->get_test_bots();
    fb::logger::debug("Starting swap test with {} bots", bots.size());

    if (bots.empty())
        throw std::runtime_error("No bots available for swap test");

    auto bot2 = bots[1];
    bot2->move(DIRECTION::RIGHT);
    bot2->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);

    fb::logger::debug("Swap test initialization completed");
    co_return;
}

async::task<void> swap_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> swap_test::on_scenario_finished(uint32_t scenario_index)
{
    co_return;
}

async::task<void> swap_test::on_parallel_scenario_started(uint32_t id)
{
    co_return;
}

async::task<void> swap_test::on_parallel_scenario_finished(uint32_t id)
{
    co_return;
}

fb::generator<bot_integration_test::scenario_t> swap_test::on_generate_scenario()
{
    auto scenarios = std::vector<std::pair<uint32_t, scenario_t>>{};

    // Bot 0: Item swap tests
    scenarios.push_back({0, [this]() -> async::task<bool> {
                             co_return co_await this->test_item_swap_1(0);
                         }});
    scenarios.push_back({0, [this]() -> async::task<bool> {
                             co_return co_await this->test_item_swap_2(0);
                         }});
    scenarios.push_back({0, [this]() -> async::task<bool> {
                             co_return co_await this->test_item_swap_3(0);
                         }});

    // Bot 1: Spell swap tests
    scenarios.push_back({1, [this]() -> async::task<bool> {
                             co_return co_await this->test_spell_swap_1(1);
                         }});
    scenarios.push_back({1, [this]() -> async::task<bool> {
                             co_return co_await this->test_spell_swap_2(1);
                         }});
    scenarios.push_back({1, [this]() -> async::task<bool> {
                             co_return co_await this->test_spell_swap_3(1);
                         }});

    co_yield [this, scenarios]() -> async::task<bool> {
        co_return co_await this->parallel_scenarios(scenarios);
    };

    co_return;
}

async::task<bool> swap_test::test_item_swap_1(uint32_t index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[index];

    fb::logger::debug("Starting item swap test 1 for bot {}", bot->name());

    // Create test items: 도토리 200개, 토끼고기 100개
    co_await bot->create_item("도토리", 200, DEFAULT_TIMEOUT);
    co_await bot->create_item("토끼고기", 100, DEFAULT_TIMEOUT);

    // Test 1: Swap items between slots 0 and 1 (both have items)
    fb::logger::debug("Test 1: Swapping items between slots 0 and 1");
    bot->chat("Starting item swap test 1: swapping 도토리 and 토끼고기");

    auto swap_request = fb::protocol::game::request::swap(SWAP_TYPE::ITEM, 1, 2);

    auto&& response = co_await bot->request<fb::protocol::game::response::item_update>(
        swap_request,
        [](auto& resp) -> bool {
            return resp.index == 0 || resp.index == 1; // Expect updates for both slots
        },
        DEFAULT_TIMEOUT);

    // Wait a bit for item swap to complete
    co_await this->sleep(DEFAULT_INTERVAL);

    if (bot->get_item_slot_by_name("도토리") != 1)
    {
        fb::logger::fatal("Item swap test 1 failed: 도토리 not found in slot 1");
        bot->chat("Item swap test 1 failed: 도토리 not found in slot 1");
        co_return false;
    }

    if (bot->get_item_slot_by_name("토끼고기") != 0)
    {
        fb::logger::fatal("Item swap test 1 failed: 토끼고기 not found in slot 0");
        bot->chat("Item swap test 1 failed: 토끼고기 not found in slot 0");
        co_return false;
    }

    bot->chat("Item swap test 1 successful");
    fb::logger::debug("Item swap test 1 completed successfully for bot {}", bot->name());
    co_return true;
}

async::task<bool> swap_test::test_item_swap_2(uint32_t index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[index];

    fb::logger::debug("Starting item swap test 2 for bot {}", bot->name());

    // Test 2: Swap slot 0 (now 토끼고기) with slot 2 (empty slot)
    fb::logger::debug("Test 2: Swapping slot 0 with empty slot 2");
    bot->chat("Starting item swap test 2: swapping with empty slot");

    auto swap_request2 = fb::protocol::game::request::swap(SWAP_TYPE::ITEM, 1, 3);

    auto&& response = co_await bot->request<fb::protocol::game::response::item_update>(
        swap_request2,
        [](auto& resp) -> bool {
            return resp.index == 0 || resp.index == 2; // Expect updates for both slots
        },
        DEFAULT_TIMEOUT);

    // Wait a bit for item swap to complete
    co_await this->sleep(DEFAULT_INTERVAL);

    // Verify empty slot after swap
    if (bot->get_item(0).has_value())
    {
        fb::logger::fatal("Item swap test 2 failed: slot 0 should be empty");
        bot->chat("Item swap test 2 failed: slot 0 should be empty");
        co_return false;
    }

    bot->chat("Item swap test 2 successful");
    fb::logger::debug("Item swap test 2 completed successfully for bot {}", bot->name());
    co_return true;
}

async::task<bool> swap_test::test_item_swap_3(uint32_t index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[index];

    fb::logger::debug("Starting item swap test 3 for bot {}", bot->name());

    // Test 3: Swap slot 0 (now empty) with slot 2 (now 토끼고기)
    fb::logger::debug("Test 3: Swapping empty slot 0 with slot 2 (토끼고기)");
    bot->chat("Starting item swap test 3: reverse empty slot swap");

    auto swap_request3 = fb::protocol::game::request::swap(SWAP_TYPE::ITEM, 1, 3);

    auto&& response = co_await bot->request<fb::protocol::game::response::item_update>(
        swap_request3,
        [](auto& resp) -> bool {
            return resp.index == 0 || resp.index == 1; // Expect updates for both slots
        },
        DEFAULT_TIMEOUT);

    // Wait a bit for item swap to complete
    co_await this->sleep(DEFAULT_INTERVAL);

    // Verify empty slot after swap
    if (bot->get_item(2).has_value())
    {
        fb::logger::fatal("Item swap test 3 failed: slot 2 should be empty");
        bot->chat("Item swap test 3 failed: slot 2 should be empty");
        co_return false;
    }

    bot->chat("Item swap test 3 successful");
    fb::logger::debug("Item swap test 3 completed successfully for bot {}", bot->name());
    co_return true;
}

async::task<bool> swap_test::test_spell_swap_1(uint32_t index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[index];

    fb::logger::debug("Starting spell swap test 1 for bot {}", bot->name());

    // Learn test spells: 누리의기원, 바다의기원
    std::ignore = co_await bot->learn_spell("누리의기원", DEFAULT_TIMEOUT);
    std::ignore = co_await bot->learn_spell("바다의기원", DEFAULT_TIMEOUT);

    // Test 1: Swap spells between slots 0 and 1 (both have spells)
    fb::logger::debug("Test 1: Swapping spells between slots 0 and 1");
    bot->chat("Starting spell swap test 1: swapping 누리의기원 and 바다의기원");

    auto spell_swap_request1 = fb::protocol::game::request::swap(SWAP_TYPE::SPELL, 1, 2);

    auto&& response = co_await bot->request<fb::protocol::game::response::spell_update>(
        spell_swap_request1,
        [](auto& resp) -> bool {
            return resp.index == 0 || resp.index == 1; // Expect updates for both slots
        },
        DEFAULT_TIMEOUT);

    // Wait a bit for spell swap to complete
    co_await this->sleep(DEFAULT_INTERVAL);

    // Verify spells are in correct slots after swap
    if (bot->get_spell_slot_by_name("누리의기원") != 1)
    {
        fb::logger::fatal("Spell swap test 1 failed: 누리의기원 not found in slot 1");
        bot->chat("Spell swap test 1 failed: 누리의기원 not found in slot 1");
        co_return false;
    }

    if (bot->get_spell_slot_by_name("바다의기원") != 0)
    {
        fb::logger::fatal("Spell swap test 1 failed: 바다의기원 not found in slot 0");
        bot->chat("Spell swap test 1 failed: 바다의기원 not found in slot 0");
        co_return false;
    }

    bot->chat("Spell swap test 1 successful");
    fb::logger::debug("Spell swap test 1 completed successfully for bot {}", bot->name());
    co_return true;
}

async::task<bool> swap_test::test_spell_swap_2(uint32_t index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[index];

    fb::logger::debug("Starting spell swap test 2 for bot {}", bot->name());

    // Test 2: Swap slot 0 (now 바다의기원) with slot 2 (empty slot)
    fb::logger::debug("Test 2: Swapping slot 0 with empty slot 2");

    auto spell_swap_request2 = fb::protocol::game::request::swap(SWAP_TYPE::SPELL, 1, 3);

    auto&& response = co_await bot->request<fb::protocol::game::response::spell_update>(
        spell_swap_request2,
        [](auto& resp) -> bool {
            return resp.index == 0 || resp.index == 2; // Expect updates for both slots
        },
        DEFAULT_TIMEOUT);

    // Wait a bit for spell swap to complete
    co_await this->sleep(DEFAULT_INTERVAL);

    // Verify spells are in correct slots after swap
    if (bot->get_spell_slot_by_name("바다의기원") != 2)
    {
        fb::logger::fatal("Spell swap test 2 failed: 바다의기원 not found in slot 2");
        co_return false;
    }

    bot->chat("Spell swap test 2 successful");
    fb::logger::debug("Spell swap test 2 completed successfully for bot {}", bot->name());
    co_return true;
}

async::task<bool> swap_test::test_spell_swap_3(uint32_t index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[index];

    fb::logger::debug("Starting spell swap test 3 for bot {}", bot->name());

    // Test 3: Swap slot 0 (now empty) with slot 2 (now 바다의기원)
    fb::logger::debug("Test 3: Swapping empty slot 0 with slot 2 (바다의기원)");

    auto spell_swap_request3 = fb::protocol::game::request::swap(SWAP_TYPE::SPELL, 1, 3);

    auto&& response = co_await bot->request<fb::protocol::game::response::spell_update>(
        spell_swap_request3,
        [](auto& resp) -> bool {
            return resp.index == 0 || resp.index == 2; // Expect updates for both slots
        },
        DEFAULT_TIMEOUT);

    // Wait a bit for spell swap to complete
    co_await this->sleep(DEFAULT_INTERVAL);

    if (bot->get_spell(2).has_value())
    {
        fb::logger::fatal("Spell swap test 3 failed: slot 0 should be empty");
        co_return false;
    }

    // Verify spells are in correct slots after swap
    if (bot->get_spell_slot_by_name("바다의기원") != 0)
    {
        fb::logger::fatal("Spell swap test 3 failed: 바다의기원 not found in slot 0");
        co_return false;
    }

    bot->chat("Spell swap test 3 successful");
    fb::logger::debug("Spell swap test 3 completed successfully for bot {}", bot->name());
    co_return true;
}

std::string swap_test::name() const
{
    return "Swap Test";
}