#include <fb/bot/integration/throw_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

throw_test::throw_test(game_bot_controller& controller) :
    bot_integration_test(controller, 2) // Spawn 2 bots for testing
{ }

async::task<void> throw_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);

    auto bots = this->get_test_bots();
    fb::logger::debug("Starting throw test with {} bots", bots.size());

    if (bots.empty())
        throw std::runtime_error("No bots available for throw test");

    fb::logger::debug("Throw test initialization completed");
    co_return;
}

async::task<void> throw_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> throw_test::on_scenario_finished(uint32_t scenario_index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[0];
    bot->chat("/아이템삭제");
    co_await this->sleep(500ms);
}

async::task<void> throw_test::on_parallel_scenario_started(uint32_t id)
{
    co_return;
}

async::task<void> throw_test::on_parallel_scenario_finished(uint32_t id)
{
    co_return;
}

fb::generator<bot_integration_test::scenario_t> throw_test::on_generate_scenario()
{
    // Step 1: Single item throw test
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_throw_single_item();
    };

    // Step 2: All items throw test
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_throw_all_items();
    };

    co_return;
}

async::task<bool> throw_test::test_throw_single_item()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    co_await bot2->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);
    co_await this->sleep(500ms);
    co_await bot2->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);

    fb::logger::debug("Starting single item throw test for bot {}", bot1->name());

    // Create test item: 도토리 201개
    co_await bot1->create_item("도토리", 201, DEFAULT_TIMEOUT);

    // Wait a bit for item to be created
    co_await this->sleep(DEFAULT_INTERVAL);

    // Test: Throw single item (all = false)
    fb::logger::debug("Test: Throwing single item");
    bot1->chat("Starting single item throw test");

    std::ignore = co_await bot1->request<fb::protocol::game::response::item_throws>(
        fb::protocol::game::request::item_throws(false, 0),
        [&](auto& resp) -> bool {
            if (resp.ch_oid != bot1->oid())
                return false;

            auto from = bot1->position();
            if (resp.from_x != from.x || resp.to_x != from.x)
                return false;

            if (resp.from_y != from.y || resp.to_y != from.y + 7)
                return false;

            return resp.look == 49366;
        },
        DEFAULT_TIMEOUT);

    if (bot1->get_item_count_by_name("도토리") != 200)
    {
        fb::logger::fatal("Single item throw test failed: bot inventory not empty after throwing all items");
        bot1->chat("Single item throw test failed: bot inventory not empty after throwing all items");
        co_return false;
    }

    bot1->chat("Single item throw test successful");
    fb::logger::debug("Single item throw test completed successfully for bot {}", bot1->name());
    co_return true;
}

async::task<bool> throw_test::test_throw_all_items()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    fb::logger::debug("Starting all items throw test for bot {}", bot1->name());

    // Move bot2 to position: down 4, left 1 (so bot1 and bot2 are in a straight line)
    co_await bot2->move(DIRECTION::BOTTOM, 4, DEFAULT_INTERVAL);
    co_await bot2->move(DIRECTION::LEFT, 1, DEFAULT_INTERVAL);
    co_await this->sleep(500ms);
    co_await bot2->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);

    // Test: Throw all items (all = true)
    fb::logger::debug("Test: Throwing all items");
    bot1->chat("Starting all items throw test");

    std::ignore = co_await bot1->request<fb::protocol::game::response::item_throws>(
        fb::protocol::game::request::item_throws(true, 0),
        [&](auto& resp) -> bool {
            if (resp.ch_oid != bot1->oid())
                return false;

            auto from = bot1->position();
            auto to   = bot2->position();
            if (resp.from_x != from.x || resp.to_x != to.x)
                return false;

            if (resp.from_y != from.y || resp.to_y != to.y - 1)
                return false;

            return resp.look == 49366;
        },
        DEFAULT_TIMEOUT);

    // Verify bot's inventory is empty after throwing all items
    if (bot1->items().size() != 0)
    {
        fb::logger::fatal("All items throw test failed: bot inventory not empty after throwing all items");
        bot1->chat("All items throw test failed: bot inventory not empty after throwing all items");
        co_return false;
    }

    bot1->chat("All items throw test successful");
    fb::logger::debug("All items throw test completed successfully for bot {}", bot1->name());
    co_return true;
}

std::string throw_test::name() const
{
    return "Throw Test";
}