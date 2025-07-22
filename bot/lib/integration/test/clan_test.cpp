#include <fb/bot/integration/clan_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/bot/integration/dialog_bot.h>
#include <fb/bot/integration/dialog_ext_bot.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

clan_test::clan_test(game_bot_controller& controller) :
    bot_integration_test(controller, 3)
{ }

async::task<void> clan_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);

    auto bots = this->get_test_bots();
    fb::logger::debug("Starting clan test with {} bots", bots.size());

    if (bots.empty())
        throw std::runtime_error("No bots available for clan test");

    co_await super::arrange_bots_in_line_formation();
}

async::task<void> clan_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> clan_test::on_scenario_finished(uint32_t scenario_index)
{
    auto bots = this->get_test_bots();
    co_return;
}

async::task<bool> clan_test::test_clan_creation()
{
    fb::logger::debug("Bot {} starting clan creation test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== CLAN CREATION TEST STARTED ===");

    auto  bots = this->get_test_bots();
    auto& bot  = bots[0];

    fb::logger::debug("Looking for NPC 낙랑 for clan creation");
    auto npc = this->controller.container.model.npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        fb::logger::fatal("Clan test failed: NPC not found");
        co_return false;
    }
    fb::logger::debug("Found NPC 낙랑, proceeding with clan creation");

    // Click on NPC to open menu dialog
    fb::logger::debug("Clicking on NPC to open menu dialog");
    std::ignore = co_await bot->request<fb::bot::integration::dialog_bot>(
        fb::protocol::game::request::click(1),
        [&npc](auto& resp) {
            return resp.type == fb::bot::integration::dialog_type::menu && resp.look == npc->look;
        },
        DEFAULT_TIMEOUT);

    // Select 1st menu and receive input dialog
    fb::logger::debug("Selecting clan creation menu option");
    std::ignore = co_await bot->request<fb::bot::integration::dialog_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::MENU,
                                            0,
                                            "",
                                            1,
                                            0,
                                            "",
                                            DIALOG_RESULT::PREV),
        [](auto& resp) {
            return resp.type == fb::bot::integration::dialog_type::input;
        },
        DEFAULT_TIMEOUT);

    // Send input dialog and receive result dialog
    fb::logger::debug("Sending clan name: {}", bot->name());
    std::ignore = co_await bot->request<fb::bot::integration::dialog_ext_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::INPUT,
                                            0,
                                            bot->name(),
                                            0,
                                            0,
                                            "",
                                            DIALOG_RESULT::PREV),
        [](auto& resp) {
            return resp.type == fb::bot::integration::dialog_ext_type::normal;
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Confirming clan creation");
    bot->send(fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::NORMAL,
                                                  1,
                                                  "",
                                                  0,
                                                  0,
                                                  "",
                                                  DIALOG_RESULT::PREV));

    fb::logger::debug("Verifying clan creation result");
    auto&& resp =
        co_await bot->request<fb::protocol::game::response::internal_info>(fb::protocol::game::request::self_info(),
                                                                           DEFAULT_TIMEOUT);

    if (resp.clan_name != bot->name())
    {
        fb::logger::fatal("Clan test failed: Clan name mismatch - expected: {}, got: {}", bot->name(), resp.clan_name);
        co_return false;
    }

    fb::logger::debug("Clan creation successful - clan name: {}", resp.clan_name);
    bot->chat("=== CLAN CREATION TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_title()
{
    fb::logger::debug("Bot {} starting clan title test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== CLAN TITLE TEST STARTED ===");

    auto  bots = this->get_test_bots();
    auto& bot  = bots[0];

    fb::logger::debug("Looking for NPC 낙랑 for clan title change");
    auto npc = this->controller.container.model.npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        fb::logger::fatal("Clan test failed: NPC not found");
        co_return false;
    }
    fb::logger::debug("Found NPC 낙랑, proceeding with title change");

    auto clan_title = std::format("{}타이틀", bot->name());
    fb::logger::debug("Setting clan title to: {}", clan_title);

    fb::logger::debug("Clicking on NPC to open menu dialog");
    std::ignore = co_await bot->request<fb::bot::integration::dialog_bot>(
        fb::protocol::game::request::click(1),
        [&npc](auto& resp) {
            return resp.type == fb::bot::integration::dialog_type::menu && resp.look == npc->look;
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Selecting clan title menu option");
    std::ignore = co_await bot->request<fb::bot::integration::dialog_ext_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::MENU,
                                            0,
                                            "",
                                            1,
                                            0,
                                            "",
                                            DIALOG_RESULT::PREV),
        [](auto& resp) {
            return resp.type == fb::bot::integration::dialog_ext_type::list;
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Navigating to title input dialog");
    std::ignore = co_await bot->request<fb::bot::integration::dialog_ext_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::LIST,
                                            0,
                                            "",
                                            0,
                                            0,
                                            "",
                                            DIALOG_RESULT::NEXT),
        [](auto& resp) {
            return resp.type == fb::bot::integration::dialog_ext_type::input_ext;
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Sending clan title: {}", clan_title);
    std::ignore = co_await bot->request<fb::bot::integration::dialog_ext_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::INPUT_EX,
                                            0x02,
                                            clan_title,
                                            0,
                                            0,
                                            "",
                                            DIALOG_RESULT::NEXT),
        [](auto& resp) {
            if (resp.type != fb::bot::integration::dialog_ext_type::normal)
                return false;

            return resp.message == "문파 칭호 변경 성공";
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Confirming title change");
    bot->send(fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::NORMAL,
                                                  1,
                                                  "",
                                                  0,
                                                  0,
                                                  "",
                                                  DIALOG_RESULT::QUIT));

    fb::logger::debug("Verifying clan title change result");
    auto&& resp =
        co_await bot->request<fb::protocol::game::response::internal_info>(fb::protocol::game::request::self_info(),
                                                                           DEFAULT_TIMEOUT);

    if (resp.clan_title != clan_title)
    {
        fb::logger::fatal("Clan test failed: Clan title mismatch - expected: {}, got: {}", clan_title, resp.clan_title);
        co_return false;
    }

    fb::logger::debug("Clan title change successful - title: {}", resp.clan_title);
    bot->chat("=== CLAN TITLE TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::invite_to_clan(std::shared_ptr<game_bot> inviter, std::shared_ptr<game_bot> invitee)
{
    fb::logger::debug("Inviter: {}, Invitee: {}", inviter->name(), invitee->name());

    fb::logger::debug("Looking for NPC 낙랑 for clan invite");
    auto npc = this->controller.container.model.npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        fb::logger::fatal("Clan test failed: NPC not found");
        co_return false;
    }
    fb::logger::debug("Found NPC 낙랑, proceeding with clan invite");

    fb::logger::debug("Clicking on NPC to open menu dialog");
    auto&& resp1 = co_await inviter->request<fb::bot::integration::dialog_bot>(
        fb::protocol::game::request::click(1),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::menu)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    if (resp1.message != "안녕하세요. 무엇을 도와드릴까요?")
    {
        fb::logger::fatal("Clan test failed: NPC response mismatch - expected: {}, got: {}",
                          "안녕하세요. 무엇을 도와드릴까요?",
                          resp1.message);
        co_return false;
    }

    fb::logger::debug("Selecting clan invite menu option");
    auto&& resp2 = co_await inviter->request<fb::bot::integration::dialog_ext_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::MENU,
                                            0,
                                            "",
                                            1,
                                            0,
                                            "",
                                            DIALOG_RESULT::PREV),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::list)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    if (resp2.message != std::format("클랜 이름 : {}", inviter->clan_name()))
    {
        fb::logger::fatal("Clan test failed: NPC response mismatch - expected: {}, got: {}",
                          std::format("클랜 이름 : {}", inviter->clan_name()),
                          resp2.message);
        co_return false;
    }

    fb::logger::debug("Navigating to invite target selection");
    auto&& resp3 = co_await inviter->request<fb::bot::integration::dialog_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::LIST,
                                            0,
                                            "",
                                            2,
                                            0,
                                            "",
                                            DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::input)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    if (resp3.message != "상대 이름 입력")
    {
        fb::logger::fatal("Clan test failed: NPC response mismatch - expected: {}, got: {}",
                          "상대 이름 입력",
                          resp3.message);
        co_return false;
    }

    fb::logger::debug("Sending invite to target: {}", invitee->name());
    auto&& resp4 = co_await inviter->request<fb::bot::integration::dialog_bot>(
        invitee,
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::INPUT,
                                            0x02,
                                            invitee->name(),
                                            0,
                                            0,
                                            "",
                                            DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::menu)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    if (resp4.message != std::format("{} 문파에 가입?", inviter->name()))
    {
        fb::logger::fatal("Clan test failed: NPC response mismatch - expected: {}, got: {}",
                          std::format("{} 문파에 가입?", inviter->name()),
                          resp4.message);
        co_return false;
    }

    fb::logger::debug("Target {} accepting clan invite", invitee->name());
    auto&& resp5 = co_await invitee->request<fb::bot::integration::dialog_ext_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::MENU,
                                            0,
                                            "",
                                            0,
                                            0,
                                            "",
                                            DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::normal)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    if (resp5.message != std::format("{} 문파에 가입됨", inviter->clan_name()))
    {
        fb::logger::fatal("Clan test failed: NPC response mismatch - expected: {}, got: {}",
                          std::format("{} 문파에 가입됨", inviter->clan_name()),
                          resp5.message);
        co_return false;
    }

    fb::logger::debug("Target {} confirming clan join", invitee->name());
    auto&& resp6 = co_await invitee->request<fb::bot::integration::dialog_ext_bot>(
        inviter,
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::NORMAL,
                                            1,
                                            "",
                                            0,
                                            0,
                                            "",
                                            DIALOG_RESULT::QUIT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::normal)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    if (resp6.message != std::format("{}가 승락함", invitee->name()))
    {
        fb::logger::fatal("Clan test failed: NPC response mismatch - expected: {}, got: {}",
                          std::format("{}가 승락함", invitee->name()),
                          resp6.message);
        co_return false;
    }

    fb::logger::debug("Clan master {} confirming invite completion", inviter->name());
    inviter->send(fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::NORMAL,
                                                      1,
                                                      "",
                                                      0,
                                                      0,
                                                      "",
                                                      DIALOG_RESULT::QUIT));

    fb::logger::debug("Verifying clan invite result by checking target's clan info");
    auto&& resp = co_await inviter->request<fb::protocol::game::response::external_info>(
        fb::protocol::game::request::click(invitee->oid()),
        [oid = invitee->oid()](auto& resp) {
            return resp.oid == oid;
        },
        DEFAULT_TIMEOUT);

    if (resp.clan_name != inviter->clan_name())
    {
        fb::logger::fatal("Clan test failed: Clan name mismatch - expected: {}, got: {}",
                          inviter->clan_name(),
                          resp.clan_name);
        co_return false;
    }

    auto clan_title = std::format("{}타이틀", resp.clan_name);
    if (resp.clan_title != clan_title)
    {
        fb::logger::fatal("Clan test failed: Clan title mismatch - expected: {}, got: {}", clan_title, resp.clan_title);
        co_return false;
    }

    fb::logger::debug("Clan invite successful - {} joined clan {}", invitee->name(), resp.clan_name);
    co_return true;
}

async::task<bool> clan_test::test_clan_invite()
{
    fb::logger::debug("Bot {} starting clan invite test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== CLAN INVITE TEST STARTED ===");

    auto  bots   = this->get_test_bots();
    auto& bot    = bots[0];
    auto& target = bots[1];

    fb::logger::debug("Clan master: {}, Target: {}", bot->name(), target->name());

    auto success = co_await this->invite_to_clan(bot, target);
    if (!success)
    {
        fb::logger::fatal("Clan invite test failed");
        co_return false;
    }

    fb::logger::debug("Clan invite successful - {} joined clan {}", target->name(), bot->clan_name());
    bot->chat("=== CLAN INVITE TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_position()
{
    fb::logger::debug("Bot {} starting clan communication test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== CLAN COMMUNICATION TEST STARTED ===");

    // TODO: Implement clan communication test
    // - Test clan chat functionality
    // - Test clan coordination features
    // - Verify communication between clan members

    fb::logger::debug("Clan communication test - not implemented yet");
    fb::logger::debug("TODO: Test clan chat functionality");
    fb::logger::debug("TODO: Test clan coordination features");
    fb::logger::debug("TODO: Verify communication between clan members");

    co_await this->sleep(1s);

    this->get_test_bots()[0]->chat("=== CLAN COMMUNICATION TEST SKIPPED (NOT IMPLEMENTED) ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_disbanding()
{
    fb::logger::debug("Bot {} starting clan disbanding test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== CLAN DISBANDING TEST STARTED ===");

    // TODO: Implement clan disbanding test
    // - Test clan disbanding scenarios
    // - Verify cleanup when clans are dissolved
    // - Test member state after disbanding

    fb::logger::debug("Clan disbanding test - not implemented yet");
    fb::logger::debug("TODO: Test clan disbanding scenarios");
    fb::logger::debug("TODO: Verify cleanup when clans are dissolved");
    fb::logger::debug("TODO: Test member state after disbanding");

    co_await this->sleep(1s);

    this->get_test_bots()[0]->chat("=== CLAN DISBANDING TEST SKIPPED (NOT IMPLEMENTED) ===");
    co_return true;
}

fb::generator<bot_integration_test::scenario_t> clan_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_clan_creation();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_clan_title();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_clan_invite();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_clan_position();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_clan_disbanding();
    };
}

std::string clan_test::name() const
{
    return "Clan Test";
}

} // namespace fb::bot::integration