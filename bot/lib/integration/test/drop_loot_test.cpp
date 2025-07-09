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
    bot_integration_test(controller)
{
    this->_controller.hook_external(this, this, &drop_loot_test::on_hook_sequence);
    this->_controller.hook_external(this, this, &drop_loot_test::on_hook_position);

    fb::logger::debug("Drop loot test constructed");
}

void drop_loot_test::reset()
{
    // Nothing to do
}

async::task<void> drop_loot_test::initialize(game_bot_controller& controller)
{
    constexpr auto REQUIRED_BOTS = 2;

    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::info("Drop loot test initializing and spawning {} bots", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Drop loot test initialization completed - {} bots spawned", REQUIRED_BOTS);
    co_return;
}

async::task<bool> drop_loot_test::execute()
{
    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    auto bots = this->get_test_bots();
    if (bots.size() < 2)
    {
        fb::logger::fatal("Need at least 2 bots for drop loot test");
        this->set_state(test_state::failed);
        co_return false;
    }

    auto& bot1 = bots[0];
    auto& bot2 = bots[1];
    co_await bot2->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);
    co_await bot2->thread()->switching();
    co_await bot2->thread()->sleep(DEFAULT_INTERVAL);
    bot2->direction(DIRECTION::BOTTOM);

    fb::logger::info("Starting drop loot test with bot1: '{}' and bot2: '{}'", bot1->name(), bot2->name());

    using scenario_fn = std::function<async::task<bool>()>;
    std::queue<scenario_fn> scenarios;
    scenarios.push([this, &bot1, &bot2] {
        return this->test_scenario_1(bot1, bot2);
    });
    scenarios.push([this, &bot1, &bot2] {
        return this->test_scenario_2(bot1, bot2);
    });
    scenarios.push([this, &bot1, &bot2] {
        return this->test_scenario_3(bot1, bot2);
    });
    scenarios.push([this, &bot1, &bot2] {
        return this->test_scenario_4(bot1, bot2);
    });
    scenarios.push([this, &bot1, &bot2] {
        return this->test_scenario_5(bot1, bot2);
    });

    int i = 1;
    while (scenarios.empty() == false)
    {
        co_await this->reset_bot_state(bot1);
        co_await this->reset_bot_state(bot2);

        auto& scenario = scenarios.front();
        if (co_await scenario() == false)
        {
            fb::logger::fatal("Drop loot test scenario {} FAILED", i);
            this->set_state(test_state::failed);
            co_return false;
        }

        fb::logger::info("Drop loot test scenario {} PASSED", i++);
        scenarios.pop();

        // Clear all items from map after each scenario
        co_await this->clear_all_items(bot1);
    }

    fb::logger::info("All drop loot test scenarios PASSED");
    this->set_state(test_state::completed);
    this->_controller.notify_test_completed(this);
    co_return true;
}

bool drop_loot_test::is_ready() const
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

void drop_loot_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    // Call base class implementation
    bot_integration_test::on_bot_connected(bot);
    fb::logger::debug("Drop loot test: Bot {} added to collection", bot->fd());
}

async::task<void> drop_loot_test::on_hook_sequence(fb::bot::game_bot&                      bot,
                                                   const fb::protocol::game::response::id& response)
{
    fb::logger::debug("Drop loot test: Bot {} received object ID {}", bot.fd(), response.oid);

    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Drop loot test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void> drop_loot_test::on_hook_position(fb::bot::game_bot&                            bot,
                                                   const fb::protocol::game::response::position& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Drop loot test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void> drop_loot_test::reset_bot_state(std::shared_ptr<game_bot>& bot)
{
    bot->chat("/아이템초기화");
    co_await bot->change_money(0, DEFAULT_TIMEOUT);
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

async::task<bool> drop_loot_test::test_scenario_1(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2)
{
    fb::logger::info("Scenario 1: Basic item drop and loot test");

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
        bot1->send(fb::protocol::game::request::item_drop(i, true));
    }

    // 4. Drop money
    bot1->send(fb::protocol::game::request::item_drop_money(50000));

    bot1->chat("Scenario 1: Dropped all items and money");

    // 5. Loot request (no boost) - should get 0 items, only money
    auto&& response =
        co_await bot1->request<fb::protocol::game::response::message>(fb::protocol::game::request::loot(false),
                                                                      DEFAULT_TIMEOUT);

    // Check that money is recovered to 50000 and no items are recovered
    if (bot1->money() != 50000 || !bot1->items().empty())
    {
        fb::logger::fatal("Scenario 1: First loot failed - money: {}, items: {}", bot1->money(), bot1->items().size());
        co_return false;
    }

    bot1->chat("Scenario 1: First loot - money recovered, no items");

    // 6. Loot request (no boost) - should get last dropped item (도토리)
    auto&& response2 =
        co_await bot1->request<fb::protocol::game::response::message>(fb::protocol::game::request::loot(false),
                                                                      DEFAULT_TIMEOUT);

    // Check that 도토리 200개 is recovered
    if (this->get_item_count(bot1, "도토리") != 200)
    {
        fb::logger::fatal("Scenario 1: Second loot failed - 도토리 count: {}", this->get_item_count(bot1, "도토리"));
        co_return false;
    }

    bot1->chat("Scenario 1: Second loot - 도토리 recovered");

    // 7. Loot boost request - should get all remaining items
    auto&& response3 =
        co_await bot1->request<fb::protocol::game::response::message>(fb::protocol::game::request::loot(true),
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

async::task<bool> drop_loot_test::test_scenario_2(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2)
{
    fb::logger::info("Scenario 2: Inventory full item loot test");

    constexpr auto CONTAINER_CAPACITY = 52;

    // 1. Fill inventory with 52 목도 items
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        co_await bot1->create_item("목도", 1, DEFAULT_TIMEOUT);
    }

    // 2. Drop 1 item
    bot1->send(fb::protocol::game::request::item_drop(0, false));

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

async::task<bool> drop_loot_test::test_scenario_3(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2)
{
    fb::logger::info("Scenario 3: Item stack overflow loot test");

    // 1. Create 200 도토리
    co_await bot1->create_item("도토리", 200, DEFAULT_TIMEOUT);

    // 2. Drop all (all = true)
    auto dotori_slot = this->get_item_slot(bot1, "도토리");
    bot1->send(fb::protocol::game::request::item_drop(dotori_slot, true));

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
    if (this->get_item_count(bot1, "도토리") != 2)
    {
        fb::logger::fatal("Scenario 3: Item count check failed - expected 2, got {}",
                          this->get_item_count(bot1, "도토리"));
        co_return false;
    }

    bot1->chat("Scenario 3: Correctly handled item stack overflow");

    co_return true;
}

async::task<bool> drop_loot_test::test_scenario_4(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2)
{
    fb::logger::info("Scenario 4: Money overflow loot test");

    // 1. Set money to 1
    co_await bot1->change_money(1, DEFAULT_TIMEOUT);

    // 2. Drop money
    bot1->send(fb::protocol::game::request::item_drop_money(1));

    // 3. Set money to max
    co_await bot1->change_money(0xFFFFFFFF, DEFAULT_TIMEOUT);

    bot1->chat("Scenario 4: Dropped 1 money, set to max money");

    // 4. Try to loot - should fail with MESSAGE_MONEY_FULL
    std::ignore = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::loot(false),
        [](auto& resp) -> bool {
            return resp.text.find(_TEXT(MESSAGE_MONEY_FULL)) != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    bot1->chat("Scenario 4: Correctly received money full message");

    co_return true;
}

async::task<bool> drop_loot_test::test_scenario_5(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2)
{
    fb::logger::info("Scenario 5: PK loot test with hellfire spell");

    // 1. Bot1 learns hellfire and sets mana to 10000
    std::vector<std::string> spell_names = {"헬파이어"};
    co_await this->learn_spells(bot1, spell_names);
    std::ignore = co_await this->setup_bot_stats(bot1, 100000, 10000, std::nullopt, std::nullopt);

    // 2. Bot2 sets health to 50
    std::ignore = co_await this->setup_bot_stats(bot2, 100000, 100000, 50, std::nullopt);

    // 3. Bot2 moves right 1 tile and sets direction to bottom
    co_await bot2->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);
    co_await bot2->thread()->switching();
    co_await bot2->thread()->sleep(DEFAULT_INTERVAL);
    bot2->direction(DIRECTION::BOTTOM);

    // 4. Bot2 creates items
    co_await bot2->create_item("도토리", 200, DEFAULT_TIMEOUT);
    co_await bot2->create_item("목도", 1, DEFAULT_TIMEOUT);
    co_await bot2->create_item("목검", 1, DEFAULT_TIMEOUT);

    // 5. Bot2 equips 목도 and check for weapon equipment message
    auto mokdo_slot = this->get_item_slot(bot2, "목도");
    std::ignore     = co_await bot2->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::item_active(mokdo_slot),
        [](auto& resp) -> bool {
            return resp.text.find("w:무기  :") == 0;
        },
        DEFAULT_TIMEOUT);

    bot1->chat("Scenario 5: Bot2 equipped weapon");
    bot2->chat("Scenario 5: Equipped weapon successfully");

    // 7. Bot1 casts hellfire on bot2
    uint8_t hellfire_slot = 1; // First spell slot
    std::ignore           = co_await bot1->request<fb::protocol::game::response::update_external>(
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

    // 8. Bot2 moves right 1 tile
    co_await bot2->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);

    // 9. Bot1 moves right 1 tile (to bot2's original position)
    co_await bot1->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);

    // 10. Bot1 loots and should get "죽은 자의 온기가 남아있습니다." message and some items
    auto&& response = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::loot(false),
        [](auto& resp) -> bool {
            return resp.text.find("죽은 자의 온기가 남아있습니다.") != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    // Check that bot1 got some items
    if (!this->has_item(bot1, "도토리") && !this->has_item(bot1, "목도") && !this->has_item(bot1, "목검"))
    {
        fb::logger::fatal("Scenario 5: PK loot failed - no items looted");
        co_return false;
    }

    bot1->chat("Scenario 5: Successfully looted items from PK");

    co_return true;
}

} // namespace fb::bot::integration