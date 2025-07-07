#include <fb/bot/integration/trade_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/bot/integration/trade_bot.h>
#include <fb/game/protocol.h>
#include <fb/model/model.h>
#include <fb/game/trade.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

trade_test::trade_test(game_bot_controller& controller) :
    bot_integration_test(controller)
{
    this->_controller.hook_external(this, this, &trade_test::on_hook_sequence);
    this->_controller.hook_external(this, this, &trade_test::on_hook_position);

    fb::logger::debug("Trade test constructed");
}

void trade_test::reset()
{
    // Nothing to do
}

async::task<void> trade_test::initialize(game_bot_controller& controller)
{
    constexpr auto REQUIRED_BOTS = 2;

    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::info("Trade test initializing and spawning {} bots", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Trade test initialization completed - {} bots spawned", REQUIRED_BOTS);
    co_return;
}

async::task<bool> trade_test::execute()
{
    constexpr auto timeout = 1h;

    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    auto bots = this->get_test_bots();
    if (bots.size() < 2)
    {
        fb::logger::fatal("Need at least 2 bots for trade test");
        this->set_state(test_state::failed);
        co_return false;
    }

    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    fb::logger::info("Starting trade test with bot1: '{}' and bot2: '{}'", bot1->name(), bot2->name());

    // Test 1: Successful trade
    if (!co_await this->test_scenario_1(bot1, bot2, timeout))
    {
        fb::logger::fatal("Trade test scenario 1 FAILED");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Trade test scenario 1 PASSED");

    // Test 2: Money overflow
    if (!co_await this->test_scenario_2(bot1, bot2, timeout))
    {
        fb::logger::fatal("Trade test scenario 2 FAILED");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Trade test scenario 2 PASSED");

    // Test 3: Item stack overflow
    if (!co_await this->test_scenario_3(bot1, bot2, timeout))
    {
        fb::logger::fatal("Trade test scenario 3 FAILED");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Trade test scenario 3 PASSED");

    // Test 4: Inventory full
    if (!co_await this->test_scenario_4(bot1, bot2, timeout))
    {
        fb::logger::fatal("Trade test scenario 4 FAILED");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Trade test scenario 4 PASSED");

    fb::logger::info("All trade test scenarios PASSED");
    this->set_state(test_state::completed);
    this->_controller.notify_test_completed(this);
    co_return true;
}

bool trade_test::is_ready() const
{
    auto bots = this->get_test_bots();
    if (bots.size() < 2)
        return false;

    for (auto& bot : bots)
    {
        if (bot->oid() == 0)
            return false;

        if (bot->position().x == 0 && bot->position().y == 0)
            return false;
    }

    return true;
}

void trade_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    // Call base class implementation
    bot_integration_test::on_bot_connected(bot);
    fb::logger::debug("Trade test: Bot {} added to collection", bot->fd());
}

async::task<void> trade_test::on_hook_sequence(fb::bot::game_bot& bot, const fb::protocol::game::response::id& response)
{
    fb::logger::debug("Trade test: Bot {} received object ID {}", bot.fd(), response.oid);

    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Trade test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void> trade_test::on_hook_position(fb::bot::game_bot&                            bot,
                                               const fb::protocol::game::response::position& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Trade test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void> trade_test::reset_bot_state(std::shared_ptr<game_bot>& bot, const fb::model::timespan& timeout)
{
    bot->chat("/아이템초기화");
    co_await bot->change_money(0, timeout);
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

async::task<bool> trade_test::test_scenario_1(std::shared_ptr<game_bot>& bot1,
                                              std::shared_ptr<game_bot>& bot2,
                                              const fb::model::timespan& timeout)
{

    co_await this->reset_bot_state(bot1, timeout);
    co_await this->reset_bot_state(bot2, timeout);

    // 2. Bot1 creates 150 도토리, 1 부적, and changes money to 10000.
    co_await bot1->create_item("도토리", 150, timeout); // slot 0
    co_await bot1->create_item("부적", 1, timeout);     // slot 1
    co_await bot1->change_money(10000, timeout);

    // 3. Bot2 creates 200 도토리, 1 양첨목봉, and changes money to 20000.
    co_await bot2->create_item("도토리", 200, timeout); // slot 0
    co_await bot2->create_item("양첨목봉", 1, timeout); // slot 1
    co_await bot2->change_money(20000, timeout);

    // 3. Start trade (Bot1 initiates trade with Bot2).
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::REQUEST, bot2->oid(), {}),
        [oid = bot2->oid()](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::dialog)
                return false;

            if (resp.dialog_oid != oid)
                return false;

            return true;
        },
        timeout);

    bot1->chat("Scenario 1: Bot1 -> Bot2 trade initiated");

    // 2-1. The 부적 is an untradable item.
    auto&& charm_response = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot2->oid(),
                                           {.index = uint8_t(1 + 1)}),
        timeout);
    bot1->chat("Scenario 1: Bot1 tried to trade a charm.");
    if (charm_response.text.find(_TEXT(MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE)) == std::string::npos)
    {
        fb::logger::fatal("Untradable item test failed: did not receive correct message.");
        co_return false;
    }

    // 4. Both bots put up all their created items (including money).
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot2->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::bundle;
        },
        timeout);
    bot1->chat("Scenario 1: Bot1 puts up an item.");
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::ITEM_COUNT,
                                           bot2->oid(),
                                           {.count = 150}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        timeout);
    bot1->chat("Scenario 1: Bot1 sets item count.");
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_MONEY,
                                           bot2->oid(),
                                           {.money = 10000}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::money;
        },
        timeout);
    bot1->chat("Scenario 1: Bot1 puts up money.");

    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot1->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::bundle;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 puts up an item.");
    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::ITEM_COUNT,
                                           bot1->oid(),
                                           {.count = 200}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 sets item count.");
    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot1->oid(),
                                           {.index = uint8_t(1 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 puts up another item.");
    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_MONEY,
                                           bot1->oid(),
                                           {.money = 20000}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::money;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 puts up money.");

    // 5. Bot2 cancels the trade.
    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::CANCEL, bot1->oid(), {}),
        [](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::close)
                return false;

            if (resp.close_message.find(_TEXT(MESSAGE_TRADE_CANCELLED_BY_ME)) == std::string::npos)
                return false;

            return true;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 cancelled the trade.");

    // 6. Bot2 initiates trade with Bot1.
    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::REQUEST, bot1->oid(), {}),
        [oid = bot1->oid()](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::dialog)
                return false;

            if (resp.dialog_oid != oid)
                return false;

            return true;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 -> Bot1 trade re-initiated.");

    // 7. Both bots put up all their created items again.
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot2->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::bundle;
        },
        timeout);
    bot1->chat("Scenario 1: Bot1 puts up an item again.");
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::ITEM_COUNT,
                                           bot2->oid(),
                                           {.count = 150}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        timeout);
    bot1->chat("Scenario 1: Bot1 sets item count again.");
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_MONEY,
                                           bot2->oid(),
                                           {.money = 10000}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::money;
        },
        timeout);
    bot1->chat("Scenario 1: Bot1 puts up money again.");

    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot1->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::bundle;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 puts up an item again.");
    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::ITEM_COUNT,
                                           bot1->oid(),
                                           {.count = 200}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 sets item count again.");
    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot1->oid(),
                                           {.index = uint8_t(1 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 puts up another item again.");
    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_MONEY,
                                           bot1->oid(),
                                           {.money = 20000}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::money;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 puts up money again.");

    // 8. Both bots lock the trade & 9. Check for trade success message.
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot2->oid(), {}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::lock;
        },
        timeout);
    bot1->chat("Scenario 1: Bot1 locked the trade.");
    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot1->oid(), {}),
        [](auto& resp) {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::close)
                return false;

            if (resp.close_message.find(_TEXT(MESSAGE_TRADE_SUCCESS)) == std::string::npos)
                return false;

            return true;
        },
        timeout);
    bot2->chat("Scenario 1: Bot2 locked the trade, completing it.");

    // 10. Verify that items and money have been swapped between Bot1 and Bot2.
    if (bot1->money() != 20000 || bot2->money() != 10000)
    {
        fb::logger::fatal("Money not swapped correctly. bot1: %d, bot2: %d", bot1->money(), bot2->money());
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

async::task<bool> trade_test::test_scenario_2(std::shared_ptr<game_bot>& bot1,
                                              std::shared_ptr<game_bot>& bot2,
                                              const fb::model::timespan& timeout)
{

    co_await this->reset_bot_state(bot1, timeout);
    co_await this->reset_bot_state(bot2, timeout);

    co_await bot1->change_money(0xFFFFFFFF, timeout);
    co_await bot2->change_money(1, timeout);

    // Start trade
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::REQUEST, bot2->oid(), {}),
        [oid = bot2->oid()](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::dialog)
                return false;

            if (resp.dialog_oid != oid)
                return false;

            return true;
        },
        timeout);

    // Bot2 puts up 1 gold.
    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_MONEY,
                                           bot1->oid(),
                                           {.money = 1}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::money;
        },
        timeout);

    // Both bots lock the trade.
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot2->oid(), {}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::lock;
        },
        timeout);

    auto fail_msg_bot2 = co_await bot2->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot1->oid(), {}),
        timeout);
    if (fail_msg_bot2.text.find(_TEXT(MESSAGE_MONEY_FULL)) == std::string::npos)
    {
        fb::logger::fatal("Scenario 2 failed: bot2 did not receive money full message.");
        co_return false;
    }

    // Verify money.
    if (bot1->money() != 0xFFFFFFFF || bot2->money() != 1)
    {
        fb::logger::fatal("Scenario 2 failed: money was changed.");
        co_return false;
    }

    co_return true;
}

async::task<bool> trade_test::test_scenario_3(std::shared_ptr<game_bot>& bot1,
                                              std::shared_ptr<game_bot>& bot2,
                                              const fb::model::timespan& timeout)
{

    co_await this->reset_bot_state(bot1, timeout);
    co_await this->reset_bot_state(bot2, timeout);

    co_await bot1->create_item("도토리", 150, timeout);
    co_await bot2->create_item("도토리", 150, timeout);

    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::REQUEST, bot2->oid(), {}),
        [oid = bot2->oid()](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::dialog)
                return false;

            if (resp.dialog_oid != oid)
                return false;

            return true;
        },
        timeout);

    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot2->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::bundle;
        },
        timeout);
    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::ITEM_COUNT,
                                           bot2->oid(),
                                           {.count = 150}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        timeout);

    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot2->oid(), {}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::lock;
        },
        timeout);

    auto fail_msg_bot2 = co_await bot2->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot1->oid(), {}),
        timeout);
    if (fail_msg_bot2.text.find("최대") == std::string::npos) // Check for message indicating max capacity
    {
        fb::logger::fatal("Scenario 3 failed: bot2 did not receive item capacity message.");
        co_return false;
    }

    if (this->get_item_count(bot1, "도토리") != 150 || this->get_item_count(bot2, "도토리") != 150)
    {
        fb::logger::fatal("Scenario 3 failed: item counts were changed.");
        co_return false;
    }

    co_return true;
}

async::task<bool> trade_test::test_scenario_4(std::shared_ptr<game_bot>& bot1,
                                              std::shared_ptr<game_bot>& bot2,
                                              const fb::model::timespan& timeout)
{
    constexpr auto CONTAINER_CAPACITY = 52;

    co_await this->reset_bot_state(bot1, timeout);
    co_await this->reset_bot_state(bot2, timeout);

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        co_await bot1->create_item("목도", 1, timeout);
    }
    co_await bot2->create_item("현철중검", 1, timeout);

    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::REQUEST, bot2->oid(), {}),
        [oid = bot2->oid()](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::trade_bot::trade_type::dialog)
                return false;

            if (resp.dialog_oid != oid)
                return false;

            return true;
        },
        timeout);

    co_await bot2->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::UP_ITEM,
                                           bot1->oid(),
                                           {.index = uint8_t(0 + 1)}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::upload;
        },
        timeout);

    co_await bot1->request<fb::bot::integration::trade_bot>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot2->oid(), {}),
        [](auto& resp) {
            return resp.type == fb::bot::integration::trade_bot::trade_type::lock;
        },
        timeout);

    auto fail_msg_bot2 = co_await bot2->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::trade(fb::protocol::game::request::trade::state::LOCK, bot1->oid(), {}),
        timeout);
    if (fail_msg_bot2.text.find(_TEXT(MESSAGE_ITEM_FULL)) == std::string::npos)
    {
        fb::logger::fatal("Scenario 4 failed: bot2 did not receive item full message.");
        co_return false;
    }

    if (this->has_item(bot1, "현철중검") || !this->has_item(bot2, "현철중검"))
    {
        fb::logger::fatal("Scenario 4 failed: items were swapped.");
        co_return false;
    }

    co_return true;
}

} // namespace fb::bot::integration