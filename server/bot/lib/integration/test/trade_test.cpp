#include <fb/bot/integration/trade_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/bot/integration/trade_bot.h>
#include <fb/game/protocol.h>
#include <fb/model/model.h>
#include <fb/game/trade.h>
#include <queue>
#include <functional>

using namespace std::chrono_literals;

namespace fb::bot::integration {

trade_test::trade_test(game_bot_controller& controller) :
    bot_integration_test(controller, 2) // Spawn 2 bots
{ }

generator<bot_integration_test::scenario_t> trade_test::on_generate_scenario()
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

    co_return;
}

async::task<void> trade_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);

    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];
    co_await bot2->move(DIRECTION::RIGHT);
    co_await this->sleep(DEFAULT_INTERVAL);
    co_await bot2->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);

    co_return;
}

async::task<void> trade_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> trade_test::on_scenario_finished(uint32_t scenario_index)
{
    for (auto& bot : this->get_test_bots())
    {
        bot->chat("/아이템초기화");
        bot->chat("/아이템삭제");
        co_await bot->change_money(0, DEFAULT_TIMEOUT);
    }

    co_return;
}

bool trade_test::has_item(const std::shared_ptr<game_bot>& bot, const std::string& name)
{
    const auto& items = bot->items();
    for (const auto& [slot, item] : items)
    {
        if (item.name.find(name) != std::string::npos)
            return true;
    }
    return false;
}

uint16_t trade_test::get_item_count(const std::shared_ptr<game_bot>& bot, const std::string& name)
{
    const auto& items = bot->items();
    for (const auto& [slot, item] : items)
    {
        if (item.name.find(name) != std::string::npos)
            return item.count;
    }
    return 0;
}

async::task<bool> trade_test::test_scenario_1()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // 2. Bot1 creates 150 도토리, 1 부적, and changes money to 10000.
    co_await bot1->create_item("도토리", 150, DEFAULT_TIMEOUT); // slot 0
    co_await bot1->create_item("부적", 1, DEFAULT_TIMEOUT);     // slot 1
    co_await bot1->change_money(10000, DEFAULT_TIMEOUT);

    // 3. Bot2 creates 200 도토리, 1 양첨목봉, and changes money to 20000.
    co_await bot2->create_item("도토리", 200, DEFAULT_TIMEOUT); // slot 0
    co_await bot2->create_item("양첨목봉", 1, DEFAULT_TIMEOUT); // slot 1
    co_await bot2->change_money(20000, DEFAULT_TIMEOUT);

    // 3. Start trade (Bot1 initiates trade with Bot2).
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::REQUEST, bot2->oid(), {}),
        [oid = bot2->oid()](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::dialog)
                return false;

            if (resp.dialog_oid != oid)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    bot1->chat("Scenario 1: Bot1 -> Bot2 trade initiated");

    // 2-1. The 부적 is an untradable item.
    auto&& charm_response = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot2->oid(),
                                           {.index = uint8_t(1 + 1)}),
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 1: Bot1 tried to trade a charm.");
    if (charm_response.text.find(_TEXT(MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE)) == std::string::npos)
    {
        fb::logger::fatal("Untradable item test failed: did not receive correct message.");
        co_return false;
    }

    // 4. Both bots put up all their created items (including money).
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot2->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::bundle;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 1: Bot1 puts up an item.");
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::ITEM_COUNT,
                                           bot2->oid(),
                                           {.count = 150}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 1: Bot1 sets item count.");
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_MONEY,
                                           bot2->oid(),
                                           {.money = 10000}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::money;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 1: Bot1 puts up money.");

    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot1->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::bundle;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 puts up an item.");
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::ITEM_COUNT,
                                           bot1->oid(),
                                           {.count = 200}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 sets item count.");
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot1->oid(),
                                           {.index = uint8_t(1 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 puts up another item.");
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_MONEY,
                                           bot1->oid(),
                                           {.money = 20000}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::money;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 puts up money.");

    // 5. Bot2 cancels the trade.
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::CANCEL, bot1->oid(), {}),
        [](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::close)
                return false;

            if (resp.close_message.find(_TEXT(MESSAGE_TRADE_CANCELLED_BY_ME)) == std::string::npos)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 cancelled the trade.");

    // 6. Bot2 initiates trade with Bot1.
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::REQUEST, bot1->oid(), {}),
        [oid = bot1->oid()](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::dialog)
                return false;

            if (resp.dialog_oid != oid)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 -> Bot1 trade re-initiated.");

    // 7. Both bots put up all their created items again.
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot2->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::bundle;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 1: Bot1 puts up an item again.");
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::ITEM_COUNT,
                                           bot2->oid(),
                                           {.count = 150}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 1: Bot1 sets item count again.");
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_MONEY,
                                           bot2->oid(),
                                           {.money = 10000}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::money;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 1: Bot1 puts up money again.");

    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot1->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::bundle;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 puts up an item again.");
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::ITEM_COUNT,
                                           bot1->oid(),
                                           {.count = 200}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 sets item count again.");
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot1->oid(),
                                           {.index = uint8_t(1 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 puts up another item again.");
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_MONEY,
                                           bot1->oid(),
                                           {.money = 20000}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::money;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 puts up money again.");

    // 8. Both bots lock the trade & 9. Check for trade success message.
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot2->oid(), {}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::lock;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 1: Bot1 locked the trade.");
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot1->oid(), {}),
        [](auto& resp) {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::close)
                return false;

            if (resp.close_message.find(_TEXT(MESSAGE_TRADE_SUCCESS)) == std::string::npos)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 1: Bot2 locked the trade, completing it.");

    auto thread = bot1->thread();
    co_await thread->sleep(DEFAULT_INTERVAL);

    // 10. Verify that items and money have been swapped between Bot1 and Bot2.
    if (bot1->money() != 20000)
    {
        fb::logger::fatal("Money not swapped correctly. bot1: {}, bot2: {}", bot1->money(), bot2->money());
        co_return false;
    }
    if (bot2->money() != 10000)
    {
        fb::logger::fatal("Money not swapped correctly. bot1: {}, bot2: {}", bot1->money(), bot2->money());
        co_return false;
    }

    if (this->has_item(bot1, "양첨목봉") == false)
    {
        fb::logger::fatal("Item not swapped correctly.");
        co_return false;
    }

    if (this->has_item(bot1, "부적") == false)
    {
        fb::logger::fatal("Item not swapped correctly.");
        co_return false;
    }

    if (this->get_item_count(bot1, "도토리") != 200)
    {
        fb::logger::fatal("Item count not swapped correctly.");
        co_return false;
    }

    if (this->get_item_count(bot2, "도토리") != 150)
    {
        fb::logger::fatal("Item count not swapped correctly.");
        co_return false;
    }

    co_return true;
}

async::task<bool> trade_test::test_scenario_2()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // 1. Set bot1's money to max and bot2's to 1.
    co_await bot1->change_money(0xFFFFFFFF, DEFAULT_TIMEOUT);
    co_await bot2->change_money(1, DEFAULT_TIMEOUT);

    // 2. Start trade.
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::REQUEST, bot2->oid(), {}),
        [oid = bot2->oid()](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::dialog)
                return false;

            if (resp.dialog_oid != oid)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 2: Trade initiated for money overflow test.");

    // 3. Bot2 puts up 1 gold.
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_MONEY,
                                           bot1->oid(),
                                           {.money = 1}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::money;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 2: Bot2 puts up 1 gold.");

    // 4. Both bots lock the trade, expecting failure.
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot2->oid(), {}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::lock;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 2: Bot1 locked the trade.");

    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot1->oid(), {}),
        [](auto& resp) {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::close)
            {
                fb::logger::fatal("Scenario 2: wrong response type, expected close but got {}", int(resp.type));
                return false;
            }

            if (resp.close_message.find(_TEXT(MESSAGE_TRADE_FAILED)) == std::string::npos)
            {
                fb::logger::fatal("Scenario 2: did not receive money full message. Got: {}", resp.close_message);
                return false;
            }

            return true;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 2: Bot2 tried to lock, trade failed as expected.");

    auto thread = bot1->thread();
    co_await thread->sleep(DEFAULT_INTERVAL);

    // 5. Verify money has not changed.
    if (bot1->money() != 0xFFFFFFFF || bot2->money() != 1)
    {
        fb::logger::fatal("Scenario 2 failed: money was changed.");
        co_return false;
    }

    co_return true;
}

async::task<bool> trade_test::test_scenario_3()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // 1. Give each bot 150 도토리.
    co_await bot1->create_item("도토리", 150, DEFAULT_TIMEOUT);
    co_await bot2->create_item("도토리", 150, DEFAULT_TIMEOUT);

    // 2. Start trade.
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::REQUEST, bot2->oid(), {}),
        [oid = bot2->oid()](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::dialog)
                return false;

            if (resp.dialog_oid != oid)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 3: Trade initiated for item stack overflow test.");

    // 3. Bot1 puts up 150 도토리.
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot2->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::bundle;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 3: Bot1 puts up 도토리.");
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::ITEM_COUNT,
                                           bot2->oid(),
                                           {.count = 150}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 3: Bot1 sets item count.");

    // 4. Both bots lock the trade, expecting failure.
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot2->oid(), {}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::lock;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 3: Bot1 locked the trade.");

    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot1->oid(), {}),
        [](auto& resp) {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::close)
            {
                fb::logger::fatal("Scenario 3: wrong response type, expected close but got {}", int(resp.type));
                return false;
            }

            if (resp.close_message.find(_TEXT(MESSAGE_TRADE_FAILED)) ==
                std::string::npos) // Check for message indicating max capacity
            {
                fb::logger::fatal("Scenario 3: did not receive item capacity message. Got: {}", resp.close_message);
                return false;
            }

            return true;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 3: Bot2 tried to lock, trade failed as expected.");

    auto thread = bot1->thread();
    co_await thread->sleep(DEFAULT_INTERVAL);

    // 5. Verify item counts have not changed.
    if (this->get_item_count(bot1, "도토리") != 150 || this->get_item_count(bot2, "도토리") != 150)
    {
        fb::logger::fatal("Scenario 3 failed: item counts were changed.");
        co_return false;
    }

    co_return true;
}

async::task<bool> trade_test::test_scenario_4()
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // 1. Fill bot1's inventory and give bot2 one item.
    co_await bot1->fill_inventory("목도", DEFAULT_TIMEOUT);
    co_await bot2->create_item("현철중검", 1, DEFAULT_TIMEOUT);
    bot1->chat("Scenario 4: Bot1 filled inventory.");

    // 2. Start trade.
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::REQUEST, bot2->oid(), {}),
        [oid = bot2->oid()](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::dialog)
                return false;

            if (resp.dialog_oid != oid)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 4: Trade initiated for inventory full test.");

    // 3. Bot2 puts up its item.
    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot1->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 4: Bot2 puts up an item.");

    // 4. Both bots lock the trade, expecting failure.
    std::ignore = co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot2->oid(), {}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::lock;
        },
        DEFAULT_TIMEOUT);
    bot1->chat("Scenario 4: Bot1 locked the trade.");

    std::ignore = co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot1->oid(), {}),
        [](auto& resp) {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::close)
            {
                fb::logger::fatal("Scenario 4: wrong response type, expected close but got {}", int(resp.type));
                return false;
            }

            if (resp.close_message.find(_TEXT(MESSAGE_TRADE_FAILED)) == std::string::npos)
            {
                fb::logger::fatal("Scenario 4: did not receive item full message. Got: {}", resp.close_message);
                return false;
            }

            return true;
        },
        DEFAULT_TIMEOUT);
    bot2->chat("Scenario 4: Bot2 tried to lock, trade failed as expected.");

    auto thread = bot1->thread();
    co_await thread->sleep(DEFAULT_INTERVAL);

    // 5. Verify items have not been swapped.
    if (this->has_item(bot1, "현철중검") || !this->has_item(bot2, "현철중검"))
    {
        fb::logger::fatal("Scenario 4 failed: items were swapped.");
        co_return false;
    }

    co_return true;
}

std::string trade_test::name() const
{
    return "Trade Test";
}

} // namespace fb::bot::integration