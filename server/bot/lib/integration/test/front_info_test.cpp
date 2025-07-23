#include <fb/bot/integration/front_info_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

front_info_test::front_info_test(game_bot_controller& controller) :
    bot_integration_test(controller, 2) // Use 2 bots
{ }

async::task<void> front_info_test::on_initialize(game_bot_controller& controller)
{
    auto bots = this->get_test_bots();

    auto& bot2 = bots[1];
    co_await bot2->move(DIRECTION::BOTTOM);
}

async::task<void> front_info_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> front_info_test::on_scenario_finished(uint32_t scenario_index)
{
    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    co_await bot1->clear_all_drop_items(DEFAULT_TIMEOUT);
    co_await this->sleep(DEFAULT_INTERVAL);
}

generator<bot_integration_test::scenario_t> front_info_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_front_info();
    };
}

std::string front_info_test::name() const
{
    return "Front Info Test";
}

async::task<bool> front_info_test::test_front_info()
{
    fb::logger::debug("Starting front_info packet handler test");

    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    bot2->chat("Creating items");
    co_await bot2->create_item("목도", 1, DEFAULT_TIMEOUT);
    co_await bot2->create_item("목검", 1, DEFAULT_TIMEOUT);
    co_await bot2->create_item("도토리", 100, DEFAULT_TIMEOUT);

    bot2->chat("Changing money to 10000");
    co_await bot2->change_money(10000, DEFAULT_TIMEOUT);

    bot2->chat("Dropping items and money");
    co_await bot2->drop_item(0, true, DEFAULT_TIMEOUT);
    co_await bot2->drop_item(1, true, DEFAULT_TIMEOUT);
    co_await bot2->drop_item(2, true, DEFAULT_TIMEOUT);
    co_await bot2->drop_money(10000, DEFAULT_TIMEOUT);

    bot1->chat("Sending front_info request");

    auto expected_messages =
        std::unordered_set<std::string>{bot2->name(), "목도", "목검", "도토리 100개", "금덩어리 10000전"};
    std::ignore = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::front_info(),
        [&expected_messages](auto& resp) -> bool {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            if (expected_messages.contains(resp.text))
                expected_messages.erase(resp.text);
            return expected_messages.empty();
        },
        DEFAULT_TIMEOUT);

    bot1->chat("Front info test completed");
    co_return true;
}