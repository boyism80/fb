#include <fb/bot/integration/chat_interaction_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/model/model.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

chat_interaction_test::chat_interaction_test(game_bot_controller& controller) :
    bot_integration_test(controller, 5) // Use 5 bots for parallel processing
{ }

async::task<void> chat_interaction_test::on_initialize(game_bot_controller& controller)
{
    co_await this->super::on_initialize(controller);
    co_await this->super::arrange_bots_in_line_formation();
}

async::task<void> chat_interaction_test::on_parallel_scenario_started(uint32_t id)
{
    co_return;
}

async::task<void> chat_interaction_test::on_parallel_scenario_finished(uint32_t id)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[id];
    bot->chat("/엔피씨제거");
    co_await this->sleep(1s);
    bot->chat(std::format("Chat interaction scenario {} completed successfully", id));
}

generator<bot_integration_test::scenario_t> chat_interaction_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->parallel_scenario_1();
    };
}

std::string chat_interaction_test::name() const
{
    return "Chat Interaction Test";
}

async::task<bool> chat_interaction_test::test_scenario_1(int index)
{
    fb::logger::debug("Starting chat interaction scenario 1 test");

    auto  bots   = this->get_test_bots();
    auto& bot    = bots[index];
    auto  passed = true;

    try
    {
        // Step 1: Bot moves down 1 step
        bot->chat("Moving down 1 step");
        co_await bot->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);
        co_await bot->move(DIRECTION::BOTTOM);

        // Step 2: Create NPC using chat command
        bot->chat("Creating NPC 왈숙네");
        auto npc     = co_await bot->create_npc("왈숙네", DEFAULT_TIMEOUT);
        auto npc_oid = npc.oid;

        co_await bot->move(DIRECTION::TOP);
        co_await bot->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);

        // Step 3: Create 진호박 10개
        bot->chat("Creating 진호박 10개");
        co_await bot->create_item("진호박", 10, DEFAULT_TIMEOUT);

        // Step 4: Sell 진호박 5개 via chat
        bot->chat("Selling 진호박 5개 via chat");
        auto base_price     = game_bot::simple_item("진호박", 0).get_price(this->controller);
        auto expected_money = (base_price / 2) * 5;

        std::ignore = co_await bot->request<game_resp::update_internal>(
            game_reqs::chat(false, "진호박 5개 판다"),
            [expected_money](auto& resp) -> bool {
                // Verify money increase
                return resp.ch_money >= expected_money;
            },
            DEFAULT_TIMEOUT);

        // Step 5: Sell remaining 진호박 5개 via chat
        bot->chat("Selling remaining 진호박 5개 via chat");
        expected_money = (base_price / 2) * 10;

        std::ignore = co_await bot->request<game_resp::update_internal>(
            game_reqs::chat(false, "진호박 다 판다"),
            [expected_money](auto& resp) -> bool {
                return resp.ch_money >= expected_money;
            },
            DEFAULT_TIMEOUT);

        // Step 6: Deposit all money via chat
        bot->chat("Depositing all money via chat");
        std::ignore = co_await bot->request<game_resp::update_internal>(
            game_reqs::chat(false, "돈 다 맡아줘"),
            [](auto& resp) -> bool {
                return resp.ch_money == 0;
            },
            DEFAULT_TIMEOUT);

        // Step 7: Withdraw all money via chat
        bot->chat("Withdrawing all money via chat");
        std::ignore = co_await bot->request<game_resp::update_internal>(
            game_reqs::chat(false, "돈 다 돌려줘"),
            [](auto& resp) -> bool {
                return resp.ch_money > 0;
            },
            DEFAULT_TIMEOUT);

        // Step 8: Get 동동주 via chat
        bot->chat("Getting 동동주 via chat");
        std::ignore = co_await bot->request<game_resp::item_update>(
            game_reqs::chat(false, "동동주 줘"),
            [](auto& resp) -> bool {
                return resp.name.find("동동주") == 0;
            },
            DEFAULT_TIMEOUT);

        // Step 9: Deposit 동동주 via chat
        bot->chat("Depositing 동동주 via chat");
        std::ignore =
            co_await bot->request<game_resp::item_remove>(game_reqs::chat(false, "동동주 맡아줘"), DEFAULT_TIMEOUT);

        // Step 10: Withdraw 동동주 via chat
        bot->chat("Withdrawing 동동주 via chat");
        std::ignore = co_await bot->request<game_resp::item_update>(
            game_reqs::chat(false, "동동주 돌려줘"),
            [](auto& resp) -> bool {
                return resp.name.find("동동주") == 0;
            },
            DEFAULT_TIMEOUT);

        // Step 11: Create 도토리 201개 and deposit all
        bot->chat("Creating 도토리 201개 and depositing all");
        co_await bot->create_item("도토리", 201, DEFAULT_TIMEOUT);

        std::ignore = co_await bot->request<game_resp::item_remove>(
            game_reqs::chat(false, "도토리 다 맡아줘"),
            [](auto& resp) -> bool {
                return true; // Just verify response received
            },
            DEFAULT_TIMEOUT);

        // Step 12: Create 도토리 201개 again and deposit all
        bot->chat("Creating 도토리 201개 again and depositing all");
        co_await bot->create_item("도토리", 201, DEFAULT_TIMEOUT);

        std::ignore = co_await bot->request<game_resp::item_remove>(
            game_reqs::chat(false, "도토리 다 맡아줘"),
            [](auto& resp) -> bool {
                return true; // Just verify response received
            },
            DEFAULT_TIMEOUT);

        // Step 13: Try to withdraw all 도토리 (should fail)
        bot->chat("Trying to withdraw all 도토리 (should fail)");
        std::ignore = co_await bot->request<game_resp::chat>(
            game_reqs::chat(false, "도토리 다 돌려줘"),
            [npc_oid](auto& resp) -> bool {
                return resp.text.find(_TEXT(MESSAGE_ITEM_CANNOT_PICKUP_ANYMORE)) != std::string::npos;
            },
            DEFAULT_TIMEOUT);
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Chat interaction scenario 1 failed: {}", e.what());
        passed = false;
    }

    co_return passed;
}

async::task<bool> chat_interaction_test::test_scenario_2(int index)
{
    fb::logger::debug("Starting chat interaction scenario 2 test");

    auto  bots   = this->get_test_bots();
    auto& bot    = bots[index];
    auto  passed = true;

    try
    {
        bot->chat("Moving down 1 step");
        co_await bot->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);
        co_await bot->move(DIRECTION::BOTTOM);

        bot->chat("Creating NPC 떡쇠");
        auto npc     = co_await bot->create_npc("떡쇠", DEFAULT_TIMEOUT);
        auto npc_oid = npc.oid;

        co_await bot->move(DIRECTION::TOP);
        co_await bot->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);

        co_await bot->create_item("양첨목봉", 1, DEFAULT_TIMEOUT);
        co_await bot->change_money(100000, DEFAULT_TIMEOUT);

        std::ignore = co_await bot->request<game_resp::chat>(
            game_reqs::chat(false, "전부 고쳐줘"),
            [npc_oid](auto& resp) -> bool {
                if (resp.oid != npc_oid)
                    return false;

                return resp.text.find(_TEXT(MESSAGE_ITEM_NOT_REPAIRABLE)) != std::string::npos;
            },
            DEFAULT_TIMEOUT);

        bot->chat("/내구도 1");
        co_await this->sleep(1s);

        std::ignore = co_await bot->request<game_resp::chat>(
            game_reqs::chat(false, "전부 고쳐줘"),
            [npc_oid](auto& resp) -> bool {
                if (resp.oid != npc_oid)
                    return false;

                return resp.text.find("고치는데 9900전이 들었습니다.") != std::string::npos;
            },
            DEFAULT_TIMEOUT);

        std::ignore = co_await bot->request<game_resp::item_update>(
            game_reqs::chat(false, "양첨목봉 이름을 채승현으로 명명"),
            [npc_oid](auto& resp) -> bool {
                if (resp.index != 0)
                    return false;

                return resp.name == "채승현";
            },
            DEFAULT_TIMEOUT);
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Chat interaction scenario 2 failed: {}", e.what());
        passed = false;
    }

    co_return passed;
}

async::task<bool> chat_interaction_test::test_scenario_3(int index)
{
    auto  bots   = this->get_test_bots();
    auto& bot    = bots[index];
    auto  passed = true;

    try
    {
        bot->chat("Moving down 1 step");
        co_await bot->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);
        co_await bot->move(DIRECTION::BOTTOM);

        bot->chat("Creating NPC 좌성황");
        auto npc     = co_await bot->create_npc("좌성황", DEFAULT_TIMEOUT);
        auto npc_oid = npc.oid;

        co_await bot->move(DIRECTION::TOP);
        co_await bot->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);

        auto spell_index = co_await bot->learn_spell("헬파이어", DEFAULT_TIMEOUT);
        fb::logger::debug("Spell index: {}", spell_index);
        co_await bot->change_mp(100000, DEFAULT_TIMEOUT);

        std::ignore = co_await bot->request<game_resp::update_internal>(
            game_reqs::spell_cast(SPELL_TYPE::TARGET, spell_index, "", bot->oid(), bot->position()),
            [](auto& resp) -> bool {
                return resp.ch_hp == 0;
            },
            DEFAULT_TIMEOUT);

        while (true)
        {
            auto&& resp = co_await bot->request<game_resp::chat>(
                game_reqs::chat(false, "살려주세요"),
                [npc_oid](auto& resp) -> bool {
                    return resp.oid == npc_oid;
                },
                DEFAULT_TIMEOUT);

            if (resp.text.find("너의 정성에 감복하여 살려줄터이니 잠시 기다려라....") != std::string::npos)
                break;
        }

        // Wait for revive
        while (bot->state() != STATE::NORMAL)
        {
            co_await this->sleep(100ms);
        }

        std::ignore = co_await bot->request<game_resp::update_internal>(
            game_reqs::chat(false, "감사합니다"),
            [bot, npc_oid](auto& resp) -> bool {
                return resp.ch_hp == bot->base_hp();
            },
            DEFAULT_TIMEOUT);
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Chat interaction scenario 3 failed: {}", e.what());
        passed = false;
    }

    co_return passed;
}

async::task<bool> chat_interaction_test::test_scenario_4(int index)
{
    auto  bots   = this->get_test_bots();
    auto& bot    = bots[index];
    auto  passed = true;

    try
    {
        bot->chat("Moving down 1 step");
        co_await bot->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);
        co_await bot->move(DIRECTION::BOTTOM);

        bot->chat("Creating NPC 뭉치");
        auto npc     = co_await bot->create_npc("뭉치", DEFAULT_TIMEOUT);
        auto npc_oid = npc.oid;

        co_await bot->move(DIRECTION::TOP);
        co_await bot->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);

        std::ignore = co_await bot->request<fb::bot::integration::dialog_ext_bot>(
            game_reqs::click(npc_oid),
            [](auto& resp) -> bool {
                return resp.type == dialog_ext_type::list;
            },
            DEFAULT_TIMEOUT);

        std::ignore = co_await bot->request<fb::bot::integration::dialog_ext_bot>(
            game_reqs::dialog(game_reqs::dialog::INTERACTION::LIST, 0, "", 0, 0, "", DIALOG_RESULT::NEXT),
            [](auto& resp) -> bool {
                return resp.type == dialog_ext_type::list;
            },
            DEFAULT_TIMEOUT);

        std::ignore = co_await bot->request<fb::bot::integration::dialog_bot>(
            game_reqs::dialog(game_reqs::dialog::INTERACTION::LIST, 0, "", 0, 0, "", DIALOG_RESULT::NEXT),
            [](auto& resp) -> bool {
                return resp.type == dialog_type::item;
            },
            DEFAULT_TIMEOUT);

        auto   item_name = "unknown";
        auto&& resp      = co_await bot->request<fb::bot::integration::dialog_ext_bot>(
            game_reqs::dialog(game_reqs::dialog::INTERACTION::ITEM, 0, item_name, 0, 0, "", DIALOG_RESULT::NEXT),
            [](auto& resp) -> bool {
                return resp.type == dialog_ext_type::normal;
            },
            DEFAULT_TIMEOUT);

        passed = resp.message == "This player is a hacker.";
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Chat interaction scenario 3 failed: {}", e.what());
        passed = false;
    }

    co_return passed;
}

async::task<bool> chat_interaction_test::parallel_scenario_1()
{
    auto bots      = this->get_test_bots();
    auto scenarios = std::vector<std::pair<uint32_t, scenario_t>>{
        {0,
         [this,    &bots]() -> async::task<bool> {
             co_return co_await this->test_scenario_1(0);
         }   },
        {1,
         [this,    &bots]() -> async::task<bool> {
             co_return co_await this->test_scenario_2(1);
         }   },
        {2,
         [this,    &bots]() -> async::task<bool> {
             co_return co_await this->test_scenario_3(2);
         }   },
        {3, [this, &bots]() -> async::task<bool> {
             co_return co_await this->test_scenario_4(3);
         }}
    };
    co_return co_await this->parallel_scenarios(scenarios);
}