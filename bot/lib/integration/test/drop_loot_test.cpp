#include <fb/bot/integration/drop_loot_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/game/protocol.h>
#include <fb/model/model.h>
#include <queue>
#include <functional>

using namespace std::chrono_literals;

namespace fb::bot::integration {

drop_loot_test::drop_loot_test(game_bot_controller& controller) :
    bot_integration_test(controller, 2)
{
    fb::logger::debug("Drop loot test constructed");
}

async::task<void> drop_loot_test::on_initialize(game_bot_controller& controller)
{
    super::on_initialize(controller);

    auto bots = this->get_test_bots();
    if (bots.size() < 2)
        throw std::runtime_error("Not enough bots for this test, requires 2.");

    auto& bot1 = bots[0];
    auto& bot2 = bots[1];
    co_await bot2->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);
    co_await bot2->thread()->sleep(DEFAULT_INTERVAL);
    co_await bot2->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);
    co_return;
}

async::task<void> drop_loot_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> drop_loot_test::on_scenario_finished(uint32_t scenario_index)
{
    for (auto& bot : this->get_test_bots())
    {
        auto thread = bot->thread();
        co_await thread->switching();

        bot->chat("/아이템삭제");
        bot->chat("/아이템초기화");
        co_await bot->change_money(0, DEFAULT_TIMEOUT);
    }

    co_return;
}

generator<bot_integration_test::scenario_t> drop_loot_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_scenario_1();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_scenario_2();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_scenario_3();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_scenario_4();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_scenario_5();
    };
}

bool drop_loot_test::has_item(const std::shared_ptr<game_bot>& bot, const std::string& name)
{
    return bot->has_item_by_name(name);
}

uint16_t drop_loot_test::get_item_count(const std::shared_ptr<game_bot>& bot, const std::string& name)
{
    return bot->get_item_count_by_name(name);
}

uint8_t drop_loot_test::get_item_slot(const std::shared_ptr<game_bot>& bot, const std::string& name)
{
    return bot->get_item_slot_by_name(name);
}

async::task<bool> drop_loot_test::test_scenario_1()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // 1. Create items (5 different items: 목도, 목검, 양첨목봉, 현철중검, 도토리 200개)
    co_await bot1->create_item("목도", 1, DEFAULT_TIMEOUT);
    co_await bot1->create_item("목검", 1, DEFAULT_TIMEOUT);
    co_await bot1->create_item("양첨목봉", 1, DEFAULT_TIMEOUT);
    co_await bot1->create_item("현철중검", 1, DEFAULT_TIMEOUT);
    co_await bot1->create_item("도토리", 200, DEFAULT_TIMEOUT);

    // 2. Set money
    co_await bot1->change_money(50000, DEFAULT_TIMEOUT);

    bot1->chat("Scenario 1: Created items and set money");

    // 3. Drop all items (all = true)
    for (int i = 0; i < 5; i++)
    {
        co_await bot1->drop_item(i, true, DEFAULT_TIMEOUT);
    }

    // 4. Drop money
    std::ignore = co_await bot1->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::item_drop_money(50000),
        [](auto& resp) -> bool {
            return resp.ch_money == 0;
        },
        DEFAULT_TIMEOUT);

    bot1->chat("Scenario 1: Dropped all items and money");

    // 5. Loot request (no boost) - should get 0 items, only money
    std::ignore = co_await bot1->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::loot(false),
        [](auto& resp) -> bool {
            return resp.ch_money == 50000;
        },
        DEFAULT_TIMEOUT);

    // Check that money is recovered to 50000 and no items are recovered
    if (!bot1->items().empty())
    {
        fb::logger::fatal("Scenario 1: First loot failed - money: {}, items: {}", bot1->money(), bot1->items().size());
        co_return false;
    }

    bot1->chat("Scenario 1: First loot - money recovered, no items");

    // 6. Loot request (no boost) - should get last dropped item (도토리)
    std::ignore = co_await bot1->request<fb::protocol::game::response::item_update>(
        fb::protocol::game::request::loot(false),
        [](auto& resp) -> bool {
            return resp.name.find("도토리") != std::string::npos && resp.count == 200;
        },
        DEFAULT_TIMEOUT);

    bot1->chat("Scenario 1: Second loot - 도토리 recovered");

    // 7. Loot boost request - should get all remaining items
    std::ignore = co_await bot1->request<fb::protocol::game::response::item_update>(
        fb::protocol::game::request::loot(true),
        [](auto& resp) -> bool {
            return resp.index == 4;
        },
        DEFAULT_TIMEOUT);

    // Check that all items are recovered
    if (!this->has_item(bot1, "목도") || !this->has_item(bot1, "목검") || !this->has_item(bot1, "양첨목봉") ||
        !this->has_item(bot1, "현철중검"))
    {
        fb::logger::fatal("Scenario 1: Third loot failed - missing items");
        co_return false;
    }

    bot1->chat("Scenario 1: All items and money recovered successfully");

    co_return true;
}

async::task<bool> drop_loot_test::test_scenario_2()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    constexpr auto CONTAINER_CAPACITY = 52;

    // 1. Fill inventory with 52 목도 items
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        co_await bot1->create_item("목도", 1, DEFAULT_TIMEOUT);
        bot1->chat(std::format("Scenario 2: Created {} 목도", i + 1));
    }

    // 2. Drop 1 item
    co_await bot1->drop_item(0, false, DEFAULT_TIMEOUT);

    // 3. Create another 목도 item
    co_await bot1->create_item("목도", 1, DEFAULT_TIMEOUT);

    bot1->chat("Scenario 2: Inventory full, dropped 1 item, created 1 more");

    // 4. Try to loot - should fail with MESSAGE_ITEM_FULL
    std::ignore = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::loot(false),
        [](auto& resp) -> bool {
            return resp.text.find(_TEXT(MESSAGE_ITEM_FULL)) != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    bot1->chat("Scenario 2: Correctly received inventory full message");

    co_return true;
}

async::task<bool> drop_loot_test::test_scenario_3()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // 1. Create 200 도토리
    co_await bot1->create_item("도토리", 200, DEFAULT_TIMEOUT);

    // 2. Drop all (all = true)
    auto dotori_slot = this->get_item_slot(bot1, "도토리");
    co_await bot1->drop_item(dotori_slot, true, DEFAULT_TIMEOUT);

    // 3. Create 2 more 도토리
    co_await bot1->create_item("도토리", 2, DEFAULT_TIMEOUT);

    bot1->chat("Scenario 3: Dropped 200 도토리, created 2 more");

    // 4. Try to loot - should fail with MESSAGE_ITEM_FULL and verify count stays at 2
    auto&& response = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::loot(false),
        [](auto& resp) -> bool {
            return resp.text.find(_TEXT(MESSAGE_ITEM_FULL)) != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    // Check that count stays at 2
    if (this->get_item_count(bot1, "도토리") != 201)
    {
        fb::logger::fatal("Scenario 3: Item count check failed - expected 2, got {}",
                          this->get_item_count(bot1, "도토리"));
        co_return false;
    }

    bot1->chat("Scenario 3: Correctly handled item stack overflow");

    co_return true;
}

async::task<bool> drop_loot_test::test_scenario_4()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // 1. Set money to 1
    co_await bot1->change_money(2, DEFAULT_TIMEOUT);

    // 2. Drop money
    co_await bot1->drop_money(2, DEFAULT_TIMEOUT);

    // 3. Set money to max
    co_await bot1->change_money(0xFFFFFFFE, DEFAULT_TIMEOUT);

    bot1->chat("Scenario 4: Dropped 1 money, set to max money");

    // 4. Try to loot - should fail with MESSAGE_MONEY_FULL
    std::ignore = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::loot(false),
        [](auto& resp) -> bool {
            return resp.text.find(_TEXT(MESSAGE_MONEY_FULL)) != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    // Check that money is recovered to 0xFFFFFFFF
    if (bot1->money() != 0xFFFFFFFF)
    {
        fb::logger::fatal("Scenario 4: Money recovery failed - money: {}", bot1->money());
        co_return false;
    }

    bot1->chat("Scenario 4: Correctly received money full message");

    co_return true;
}

async::task<bool> drop_loot_test::test_scenario_5()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // 1. Bot1 learns hellfire and sets mana to 10000
    auto spell_names = std::vector<std::string>{"헬파이어"};
    std::ignore      = co_await bot1->learn_spells(spell_names, DEFAULT_TIMEOUT);
    std::ignore      = co_await bot1->setup_bot_stats(100000, 10000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);

    // 2. Bot2 sets health to 50
    std::ignore = co_await bot2->setup_bot_stats(100000, 100000, 50, std::nullopt, DEFAULT_TIMEOUT);

    // 3. Bot2 moves right 1 tile and sets direction to bottom
    co_await bot2->thread()->switching();
    co_await bot2->thread()->sleep(DEFAULT_INTERVAL);
    co_await bot2->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);

    // 4. Bot2 creates items
    co_await bot2->create_item("도토리", 200, DEFAULT_TIMEOUT);
    co_await bot2->create_item("목도", 1, DEFAULT_TIMEOUT);
    co_await bot2->create_item("목검", 1, DEFAULT_TIMEOUT);

    // 5. Bot2 equips 목도 and check for weapon equipment message
    auto mokdo_slot = this->get_item_slot(bot2, "목도");
    std::ignore     = co_await bot2->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::item_active(mokdo_slot + 1),
        [](auto& resp) -> bool {
            return resp.text.find("w:무기  :") == 0;
        },
        DEFAULT_TIMEOUT);

    bot1->chat("Scenario 5: Bot2 equipped weapon");
    bot2->chat("Scenario 5: Equipped weapon successfully");

    // 7. Bot1 casts hellfire on bot2
    uint8_t hellfire_slot = 1; // First spell slot
    std::ignore           = co_await bot1->request<fb::protocol::game::response::update_external<true>>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, hellfire_slot, "", bot2->oid(), bot2->position()),
        [bot2_oid = bot2->oid()](auto& resp) -> bool {
            if (resp.oid != bot2_oid)
                return false;
            // Check if bot2 is in GHOST state
            return resp.state == STATE::GHOST;
        },
        DEFAULT_TIMEOUT);

    bot1->chat("Scenario 5: Cast hellfire on bot2");
    bot2->chat("Scenario 5: Killed by hellfire");

    co_await bot2->thread()->switching();
    co_await bot2->thread()->sleep(500ms);

    // 8. Bot2 moves right 1 tile
    co_await bot2->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);

    // 9. Bot1 moves right 1 tile (to bot2's original position)
    co_await bot1->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);

    // 10. Bot1 loots and should get "죽은 자의 온기가 남아있습니다." message and some items
    std::ignore = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::loot(false),
        [](auto& resp) -> bool {
            return resp.text.find("죽은 자의 온기가 남아있습니다.") != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    // Check that bot1 got some items
    if (bot1->items().size() > 0)
    {
        fb::logger::fatal("Scenario 5: PK loot failed - no items looted");
        co_return false;
    }

    bot1->chat("Scenario 5: Successfully looted items from PK");

    co_return true;
}

std::string drop_loot_test::name() const
{
    return "Drop Loot Test";
}

} // namespace fb::bot::integration