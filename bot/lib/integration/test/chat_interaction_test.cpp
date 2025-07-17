#include <fb/bot/integration/chat_interaction_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

chat_interaction_test::chat_interaction_test(game_bot_controller& controller) :
    bot_integration_test(controller, 5) // Use 5 bots for parallel processing
{ }

async::task<void> chat_interaction_test::on_initialize(game_bot_controller& controller)
{
    co_await this->super::on_initialize(controller);
}

generator<bot_integration_test::scenario_t> chat_interaction_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_scenario_1();
    };

    co_return;
}

std::string chat_interaction_test::name() const
{
    return "Chat Interaction Test";
}

async::task<bool> chat_interaction_test::test_scenario_1()
{
    fb::logger::debug("Starting chat interaction scenario 1 test");

    auto  bots   = this->get_test_bots();
    auto& bot1   = bots[0]; // Bot 1 executes scenario 1
    auto  passed = true;

    try
    {
        // Step 1: Bot moves down 1 step
        bot1->chat("Moving down 1 step");
        co_await bot1->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);
        co_await bot1->move(DIRECTION::BOTTOM, 1, 0ms);

        // Step 2: Create NPC using chat command
        bot1->chat("Creating NPC 왈숙네");
        auto npc     = co_await bot1->create_npc("왈숙네", DEFAULT_TIMEOUT);
        auto npc_oid = npc.oid;

        // Step 3: Create 진호박 10개
        bot1->chat("Creating 진호박 10개");
        co_await bot1->create_item("진호박", 10, DEFAULT_TIMEOUT);

        // Step 4: Sell 진호박 5개 via chat
        bot1->chat("Selling 진호박 5개 via chat");
        auto base_price     = game_bot::simple_item("진호박", 0).get_price(this->controller);
        auto expected_money = (base_price / 2) * 5;

        std::ignore = co_await bot1->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat(false, "진호박 5개 판다"),
            [expected_money](auto& resp) -> bool {
                // Verify money increase
                return resp.ch_money >= expected_money;
            },
            DEFAULT_TIMEOUT);

        // Step 5: Sell remaining 진호박 5개 via chat
        bot1->chat("Selling remaining 진호박 5개 via chat");
        expected_money = (base_price / 2) * 10;

        std::ignore = co_await bot1->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat(false, "진호박 다 판다"),
            [expected_money](auto& resp) -> bool {
                return resp.ch_money >= expected_money;
            },
            DEFAULT_TIMEOUT);

        // Step 6: Deposit all money via chat
        bot1->chat("Depositing all money via chat");
        std::ignore = co_await bot1->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat(false, "돈 다 맡아줘"),
            [](auto& resp) -> bool {
                return resp.ch_money == 0;
            },
            DEFAULT_TIMEOUT);

        // Step 7: Withdraw all money via chat
        bot1->chat("Withdrawing all money via chat");
        std::ignore = co_await bot1->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat(false, "돈 다 돌려줘"),
            [](auto& resp) -> bool {
                return resp.ch_money > 0;
            },
            DEFAULT_TIMEOUT);

        // Step 8: Get 동동주 via chat
        bot1->chat("Getting 동동주 via chat");
        std::ignore = co_await bot1->request<fb::protocol::game::response::item_update>(
            fb::protocol::game::request::chat(false, "동동주 줘"),
            [](auto& resp) -> bool {
                return resp.name.find("동동주") == 0;
            },
            DEFAULT_TIMEOUT);

        // Step 9: Deposit 동동주 via chat
        bot1->chat("Depositing 동동주 via chat");
        std::ignore = co_await bot1->request<fb::protocol::game::response::item_remove>(
            fb::protocol::game::request::chat(false, "동동주 맡아줘"),
            [](auto& resp) -> bool {
                return true; // Just verify response received
            },
            DEFAULT_TIMEOUT);

        // Step 10: Withdraw 동동주 via chat
        bot1->chat("Withdrawing 동동주 via chat");
        std::ignore = co_await bot1->request<fb::protocol::game::response::item_update>(
            fb::protocol::game::request::chat(false, "동동주 돌려줘"),
            [](auto& resp) -> bool {
                return resp.name.find("동동주") == 0;
            },
            DEFAULT_TIMEOUT);

        // Step 11: Create 도토리 201개 and deposit all
        bot1->chat("Creating 도토리 201개 and depositing all");
        co_await bot1->create_item("도토리", 201, DEFAULT_TIMEOUT);

        std::ignore = co_await bot1->request<fb::protocol::game::response::item_remove>(
            fb::protocol::game::request::chat(false, "도토리 다 맡아줘"),
            [](auto& resp) -> bool {
                return true; // Just verify response received
            },
            DEFAULT_TIMEOUT);

        // Step 12: Create 도토리 201개 again and deposit all
        bot1->chat("Creating 도토리 201개 again and depositing all");
        co_await bot1->create_item("도토리", 201, DEFAULT_TIMEOUT);

        std::ignore = co_await bot1->request<fb::protocol::game::response::item_remove>(
            fb::protocol::game::request::chat(false, "도토리 다 맡아줘"),
            [](auto& resp) -> bool {
                return true; // Just verify response received
            },
            DEFAULT_TIMEOUT);

        // Step 13: Try to withdraw all 도토리 (should fail)
        bot1->chat("Trying to withdraw all 도토리 (should fail)");
        std::ignore = co_await bot1->request<fb::protocol::game::response::chat>(
            fb::protocol::game::request::chat(false, "도토리 다 돌려줘"),
            [npc_oid](auto& resp) -> bool {
                return resp.oid == npc_oid && resp.text.find("왈숙네: 더 이상 가질 수 없습니다.") != std::string::npos;
            },
            DEFAULT_TIMEOUT);

        // Step 14: Remove NPC via chat command
        bot1->chat("Removing NPC via chat command");
        std::ignore = co_await bot1->request<fb::protocol::game::response::action>(
            fb::protocol::game::request::chat(false, "/엔피씨제거"),
            [](auto& resp) -> bool {
                return true; // Just verify response received
            },
            DEFAULT_TIMEOUT);

        bot1->chat("Chat interaction scenario 1 completed successfully");
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Chat interaction scenario 1 failed: {}", e.what());
        passed = false;
    }

    co_return passed;
}