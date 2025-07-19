#include <fb/bot/integration/item_test.give.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

item_test_give::item_test_give(game_bot_controller& controller) :
    bot_integration_test(controller, 2)
{ }

async::task<void> item_test_give::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);
    co_await super::arrange_bots_in_line_formation();
}

async::task<void> item_test_give::on_scenario_started(uint32_t scenario_index)
{
    auto bots = this->get_test_bots();
    for (auto& bot : bots)
    {
        fb::logger::debug("Resetting bot {} state to clean initial conditions", bot->name());

        co_await bot->clear_all_drop_items(DEFAULT_TIMEOUT);
        co_await bot->clear_inventory(DEFAULT_TIMEOUT);
        co_await bot->change_money(0, DEFAULT_TIMEOUT);
    }
}

async::task<void> item_test_give::on_scenario_finished(uint32_t scenario_index)
{
    co_return;
}

async::task<void> item_test_give::on_parallel_scenario_started(uint32_t id)
{
    co_return;
}

async::task<void> item_test_give::on_parallel_scenario_finished(uint32_t id)
{
    co_return;
}

generator<bot_integration_test::scenario_t> item_test_give::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_give_non_tradeable_item();
    };

    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_give_tradeable_item();
    };

    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_give_item_inventory_full();
    };

    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_give_max_money();
    };

    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_give_to_mob_and_kill();
    };

    co_return;
}

std::string item_test_give::name() const
{
    return "Item Test Give";
}

async::task<bool> item_test_give::test_give_non_tradeable_item()
{
    fb::logger::debug("Starting test: Give non-tradeable item (초심자의노리개)");

    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    co_await bot1->direction(DIRECTION::RIGHT, DEFAULT_TIMEOUT);
    co_await bot2->direction(DIRECTION::LEFT, DEFAULT_TIMEOUT);

    // Give bot1 a non-tradeable item (초심자의노리개)
    co_await bot1->create_item("초심자의노리개", 1, DEFAULT_TIMEOUT);

    // Try to give the item to bot2 (should fail)
    auto&& resp = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::give_item(0, false),
        [](auto& resp) -> bool {
            return resp.type == MESSAGE_TYPE::STATE;
        },
        DEFAULT_TIMEOUT);

    // Check if the operation failed as expected
    if (resp.text != "줄 수 없습니다.")
    {
        fb::logger::fatal("Test failed: Non-tradeable item was given successfully");
        co_return false;
    }

    // Verify bot1 still has the item
    if (!bot1->has_item_by_name("초심자의노리개"))
    {
        fb::logger::fatal("Test failed: Bot1 lost the non-tradeable item unexpectedly");
        co_return false;
    }
    co_await this->sleep(DEFAULT_INTERVAL);
    if (bot2->has_item_by_name("초심자의노리개"))
    {
        fb::logger::fatal("Test failed: Bot2 received the non-tradeable item");
        co_return false;
    }

    fb::logger::debug("Test passed: Non-tradeable item give operation failed as expected");
    co_return true;
}

async::task<bool> item_test_give::test_give_tradeable_item()
{
    fb::logger::debug("Starting test: Give tradeable item (목도)");

    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // Give bot1 a tradeable item (목도)
    co_await bot1->create_item("목도", 1, DEFAULT_TIMEOUT);

    std::ignore = co_await bot1->request<fb::protocol::game::response::item_remove>(
        fb::protocol::game::request::give_item(0, false),
        [](auto& resp) -> bool {
            return resp.index == 0;
        },
        DEFAULT_TIMEOUT);

    if (bot1->has_item_by_name("목도"))
    {
        fb::logger::fatal("Test failed: Bot1 still has the tradeable item after giving");
        co_return false;
    }

    co_await this->sleep(DEFAULT_INTERVAL);
    if (!bot2->has_item_by_name("목도"))
    {
        fb::logger::fatal("Test failed: Bot2 did not receive the tradeable item");
        co_return false;
    }

    fb::logger::debug("Test passed: Tradeable item give operation succeeded as expected");
    co_return true;
}

async::task<bool> item_test_give::test_give_item_inventory_full()
{
    fb::logger::debug("Starting test: Give item when target inventory is full");

    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // Fill bot2's inventory with items
    co_await bot2->fill_inventory("목도", DEFAULT_TIMEOUT);

    // Give bot1 a tradeable item
    co_await bot1->create_item("목도", 1, DEFAULT_TIMEOUT);

    // Try to give the item to bot2 (should fail due to full inventory)
    std::ignore = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::give_item(0, false),
        [](auto& resp) -> bool {
            return resp.type == MESSAGE_TYPE::STATE && resp.text == "상대방의 인벤토리가 가득 찼습니다.";
        },
        DEFAULT_TIMEOUT);

    if (!bot1->has_item_by_name("목도"))
    {
        fb::logger::fatal("Test failed: Item was given successfully despite full inventory");
        co_return false;
    }

    co_await bot2->drop_item(0, false, DEFAULT_TIMEOUT);
    co_await bot1->create_item("도토리", 3, DEFAULT_TIMEOUT);
    co_await bot2->create_item("도토리", 200, DEFAULT_TIMEOUT);

    std::ignore = co_await bot1->request<fb::protocol::game::response::item_update>(
        fb::protocol::game::request::give_item(1, false),
        [](auto& resp) -> bool {
            return resp.index == 1;
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::give_item(1, true),
        [](auto& resp) -> bool {
            return resp.type == MESSAGE_TYPE::STATE && resp.text == "상대방의 인벤토리가 가득 찼습니다.";
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Test passed: Item give operation failed as expected when target inventory was full");
    co_return true;
}

async::task<bool> item_test_give::test_give_max_money()
{
    fb::logger::debug("Starting test: Give maximum money amount (0xFFFFFFFE)");

    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // Give bot1 maximum money
    co_await bot1->change_money(0xFFFFFFFE, DEFAULT_TIMEOUT);

    // Try to give maximum money to bot2 (should succeed)
    std::ignore = co_await bot1->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::give_money(0xFFFFFFFE),
        [](auto& resp) -> bool {
            return resp.ch_money == 0;
        },
        DEFAULT_TIMEOUT);

    co_await this->sleep(DEFAULT_INTERVAL);

    // Verify bot2 now has the money
    if (bot2->money() != 0xFFFFFFFE)
    {
        fb::logger::fatal("Test failed: Bot2 did not receive the maximum money amount");
        co_return false;
    }

    co_await bot1->change_money(2, DEFAULT_TIMEOUT);

    std::ignore = co_await bot1->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::give_money(2),
        [](auto& resp) -> bool {
            return resp.ch_money == 1;
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::give_money(2),
        [](auto& resp) -> bool {
            return resp.type == MESSAGE_TYPE::STATE && resp.text == "상대방이 돈을 받을 수 없습니다.";
        },
        DEFAULT_TIMEOUT);

    if (bot1->money() != 1)
    {
        fb::logger::fatal("Test failed: Bot1 did not receive the money amount");
        co_return false;
    }

    if (bot2->money() != 0xFFFFFFFF)
    {
        fb::logger::fatal("Test failed: Bot2 did not receive the maximum money amount");
        co_return false;
    }

    fb::logger::debug("Test passed: Maximum money give operation succeeded as expected");
    co_return true;
}

async::task<bool> item_test_give::test_give_to_mob_and_kill()
{
    fb::logger::debug("Starting test: Give item and money to mob then kill it");

    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    co_await bot1->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);
    co_await bot1->learn_spell("헬파이어", DEFAULT_TIMEOUT);
    co_await bot1->learn_spell("노획", DEFAULT_TIMEOUT);

    // Give bot1 items and money
    co_await bot1->create_item("목도", 1, DEFAULT_TIMEOUT);
    co_await bot1->change_money(1000, DEFAULT_TIMEOUT);
    co_await bot1->set_max_hp_mp(1000, 1000, DEFAULT_TIMEOUT);

    // Spawn a mob below bot1
    auto mob_position  = bot1->position();
    mob_position.y    += 1; // Position below bot1

    auto mob_info = co_await bot1->spawn_monster("다람쥐", mob_position.x, mob_position.y, DEFAULT_TIMEOUT);
    if (mob_info.oid == 0)
    {
        fb::logger::fatal("Test failed: Failed to spawn mob");
        co_return false;
    }

    // Give item to mob (should succeed)
    std::ignore = co_await bot1->request<fb::protocol::game::response::item_remove>(
        fb::protocol::game::request::give_item(0, false),
        [](auto& resp) -> bool {
            return resp.index == 0;
        },
        DEFAULT_TIMEOUT);

    // Give money to mob (should succeed)
    std::ignore = co_await bot1->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::give_money(500),
        [](auto& resp) -> bool {
            return resp.ch_money == 500;
        },
        DEFAULT_TIMEOUT);

    if (bot1->money() != 500)
    {
        fb::logger::fatal("Test failed: Failed to give money to mob");
        co_return false;
    }

    // Cast spell to kill the mob
    std::ignore = co_await bot1->request<fb::protocol::game::response::update>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, 0, "", mob_info.oid, mob_info.position),
        [mob_info](auto& resp) -> bool {
            if (resp.objects_data.size() < 2)
                return false;

            for (auto& object : resp.objects_data)
            {
                if (object.x != mob_info.position.x || object.y != mob_info.position.y)
                    return false;
            }

            return true;
        },
        DEFAULT_TIMEOUT);

    co_await bot1->change_mp(1000, DEFAULT_TIMEOUT);
    std::ignore = co_await bot1->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, 1, "", mob_info.oid, mob_info.position),
        [](auto& resp) -> bool {
            return resp.ch_money == 1000;
        },
        DEFAULT_TIMEOUT);

    co_await this->sleep(DEFAULT_INTERVAL);
    if (!bot1->has_item_by_name("목도"))
    {
        fb::logger::fatal("Test failed: Bot1 did not receive the item after giving to mob");
        co_return false;
    }

    fb::logger::debug("Test passed: Successfully gave item and money to mob then killed it");
    co_return true;
}

} // namespace fb::bot::integration