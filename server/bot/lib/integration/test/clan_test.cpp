#include <fb/bot/integration/clan_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/bot/integration/dialog_bot.h>
#include <fb/bot/integration/dialog_ext_bot.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

clan_test::clan_test(game_bot_controller& controller) :
    bot_integration_test(controller, 4)
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
    fb::logger::debug("Clan test scenario {} finished", scenario_index);
    co_return;
}

async::task<bool> clan_test::test_clan_creation()
{
    fb::logger::debug("Bot {} starting clan creation test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== CLAN CREATION TEST STARTED ===");

    auto  bots = this->get_test_bots();
    auto& bot  = bots[0];

    fb::logger::debug("Looking for NPC 낙랑 for clan creation");
    auto npc = table::npc.name2npc("낙랑");
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
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::MENU, 0, "", 1, 0, "", DIALOG_RESULT::PREV),
        [](auto& resp) {
            return resp.type == fb::bot::integration::dialog_type::input;
        },
        DEFAULT_TIMEOUT);

    // Send input dialog and receive result dialog
    fb::logger::debug("Sending clan name: {}", bot->name());
    std::ignore = co_await bot->request<fb::bot::integration::dialog_ext_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::INPUT, 0, bot->name(), 0, 0, "", DIALOG_RESULT::PREV),
        [](auto& resp) {
            return resp.type == fb::bot::integration::dialog_ext_type::normal;
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Confirming clan creation");
    bot->send(fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::NORMAL, 1, "", 0, 0, "", DIALOG_RESULT::PREV));

    fb::logger::debug("Verifying clan creation result");
    auto&& resp = co_await bot->request<fb::protocol::game::response::internal_info>(fb::protocol::game::request::self_info(), DEFAULT_TIMEOUT);

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
    auto npc = table::npc.name2npc("낙랑");
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
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::MENU, 0, "", 1, 0, "", DIALOG_RESULT::PREV),
        [](auto& resp) {
            return resp.type == fb::bot::integration::dialog_ext_type::list;
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Navigating to title input dialog");
    std::ignore = co_await bot->request<fb::bot::integration::dialog_ext_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::LIST, 0, "", 0, 0, "", DIALOG_RESULT::NEXT),
        [](auto& resp) {
            return resp.type == fb::bot::integration::dialog_ext_type::input_ext;
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Sending clan title: {}", clan_title);
    std::ignore = co_await bot->request<fb::bot::integration::dialog_ext_bot>(
        fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::INPUT_EX, 0x02, clan_title, 0, 0, "", DIALOG_RESULT::NEXT),
        [](auto& resp) {
            if (resp.type != fb::bot::integration::dialog_ext_type::normal)
                return false;

            return resp.message == "문파 칭호 변경 성공";
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Confirming title change");
    bot->send(fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::NORMAL, 1, "", 0, 0, "", DIALOG_RESULT::QUIT));

    fb::logger::debug("Verifying clan title change result");
    auto&& resp = co_await bot->request<fb::protocol::game::response::internal_info>(fb::protocol::game::request::self_info(), DEFAULT_TIMEOUT);

    if (resp.clan_title != clan_title)
    {
        fb::logger::fatal("Clan test failed: Clan title mismatch - expected: {}, got: {}", clan_title, resp.clan_title);
        co_return false;
    }

    fb::logger::debug("Clan title change successful - title: {}", resp.clan_title);
    bot->chat("=== CLAN TITLE TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_invite()
{
    fb::logger::debug("Bot {} starting clan invite test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== CLAN INVITE TEST STARTED ===");

    auto  bots   = this->get_test_bots();
    auto& bot    = bots[0];
    auto& target = bots[1];

    fb::logger::debug("Clan invite test setup - Clan master: {}, Target: {}", bot->name(), target->name());
    fb::logger::debug("Testing invitation process from {} to {}", bot->name(), target->name());

    auto success = co_await bot->invite_to_clan(target, DEFAULT_TIMEOUT);
    if (!success)
    {
        fb::logger::fatal("Clan invite test failed: Invitation process unsuccessful");
        bot->chat("=== CLAN INVITE TEST FAILED: Invitation process failed ===");
        co_return false;
    }

    fb::logger::debug("Clan invite test completed successfully - {} successfully joined clan {}", target->name(), bot->clan_name());
    bot->chat("=== CLAN INVITE TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_role()
{
    auto bots = this->get_test_bots();
    fb::logger::debug("Bot {} starting clan role test", bots[0]->oid());
    bots[0]->chat("=== CLAN ROLE TEST STARTED ===");

    fb::logger::debug("Testing role-based invite permissions - {} trying to invite {}", bots[1]->name(), bots[2]->name());
    if (co_await bots[1]->invite_to_clan(bots[2], DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan role test failed: {} should not be able to invite {} without proper role", bots[1]->name(), bots[2]->name());
        bots[0]->chat("=== CLAN ROLE TEST FAILED: Invalid invite permission ===");
        co_return false;
    }
    fb::logger::debug("Role permission test passed: {} correctly denied invite permission", bots[1]->name());

    fb::logger::debug("Looking for NPC 낙랑 for role change test");
    auto npc = table::npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        fb::logger::fatal("Clan test failed: NPC not found");
        bots[0]->chat("=== CLAN ROLE TEST FAILED: NPC not found ===");
        co_return false;
    }
    fb::logger::debug("Found NPC 낙랑, proceeding with role change test");

    fb::logger::debug("Changing {} role to OFFICER (role ID: 1)", bots[1]->name());
    if (!co_await bots[0]->change_clan_role(bots[1], fb::model::const_value::clan::MINIMUM_INVITE_PRIVILEGE, DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan role test failed: Role change unsuccessful");
        bots[0]->chat("=== CLAN ROLE TEST FAILED: Role change unsuccessful ===");
        co_return false;
    }
    fb::logger::debug("Role change successful: {} promoted to OFFICER", bots[1]->name());

    fb::logger::debug("Testing invite permission after role change - {} should now be able to invite {}", bots[1]->name(), bots[2]->name());
    if (!co_await bots[1]->invite_to_clan(bots[2], DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan role test failed: {} should be able to invite {} after role promotion", bots[1]->name(), bots[2]->name());
        bots[0]->chat("=== CLAN ROLE TEST FAILED: Invite permission not granted after role change ===");
        co_return false;
    }

    if (co_await bots[1]->change_clan_role(bots[2], CLAN_ROLE::MASTER, DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan role test failed: {} should not be able to promote {} to OFFICER", bots[1]->name(), bots[2]->name());
        bots[0]->chat("=== CLAN ROLE TEST FAILED: Invalid role promotion ===");
        co_return false;
    }

    if (co_await bots[1]->change_clan_role(bots[3], CLAN_ROLE::MATE, DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan role test failed: {} should not be able to promote {} to OFFICER", bots[1]->name(), bots[3]->name());
        bots[0]->chat("=== CLAN ROLE TEST FAILED: Invalid role promotion ===");
        co_return false;
    }

    fb::logger::debug("Role-based invite permission test passed: {} successfully invited {} after role promotion", bots[1]->name(), bots[2]->name());

    fb::logger::debug("Clan role test completed successfully");
    bots[0]->chat("=== CLAN ROLE TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_disbanding()
{
    fb::logger::debug("Bot {} starting clan disbanding test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== CLAN DISBANDING TEST STARTED ===");

    auto bots = this->get_test_bots();

    // Verify initial clan setup: bot0=master, bot1=deputy, bot2=mate
    fb::logger::debug("Verifying initial clan setup - Master: {}, Deputy: {}, Mate: {}", bots[0]->name(), bots[1]->name(), bots[2]->name());

    // Step 1: Bot2 (mate) tries to kick bot1 (deputy) - should fail
    fb::logger::debug("Step 1: {} (mate) attempting to kick {} (deputy) - should fail", bots[2]->name(), bots[1]->name());

    if (co_await bots[2]->kick_from_clan(bots[1], DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan disbanding test failed: {} (mate) should not be able to kick {} (deputy)", bots[2]->name(), bots[1]->name());
        bots[0]->chat("=== CLAN DISBANDING TEST FAILED: Mate should not be able to kick deputy ===");
        co_return false;
    }
    fb::logger::debug("Step 1 passed: {} (mate) correctly failed to kick {} (deputy)", bots[2]->name(), bots[1]->name());

    // Step 2: Bot2 (mate) leaves clan - should succeed
    fb::logger::debug("Step 2: {} (mate) leaving clan - should succeed", bots[2]->name());

    if (!co_await bots[2]->leave_clan(DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan disbanding test failed: {} (mate) should be able to leave clan", bots[2]->name());
        bots[0]->chat("=== CLAN DISBANDING TEST FAILED: Mate should be able to leave clan ===");
        co_return false;
    }
    fb::logger::debug("Step 2 passed: {} (mate) successfully left clan", bots[2]->name());

    // Step 3: Bot0 (master) kicks bot1 (deputy) - should succeed
    fb::logger::debug("Step 3: {} (master) kicking {} (deputy) - should succeed", bots[0]->name(), bots[1]->name());

    if (!co_await bots[0]->kick_from_clan(bots[1], DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan disbanding test failed: {} (master) should be able to kick {} (deputy)", bots[0]->name(), bots[1]->name());
        bots[0]->chat("=== CLAN DISBANDING TEST FAILED: Master should be able to kick deputy ===");
        co_return false;
    }
    fb::logger::debug("Step 3 passed: {} (master) successfully kicked {} (deputy)", bots[0]->name(), bots[1]->name());

    // Step 4: Bot0 (master) is now the only member, should destroy clan
    fb::logger::debug("Step 4: {} (master) is now the only member, destroying clan", bots[0]->name());

    if (!co_await bots[0]->destroy_clan(DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan disbanding test failed: {} (master) should be able to destroy clan", bots[0]->name());
        bots[0]->chat("=== CLAN DISBANDING TEST FAILED: Master should be able to destroy clan ===");
        co_return false;
    }
    fb::logger::debug("Step 4 passed: {} (master) successfully destroyed clan", bots[0]->name());

    fb::logger::debug("Clan disbanding test completed successfully");
    bots[0]->chat("=== CLAN DISBANDING TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_title_change()
{
    fb::logger::debug("Bot {} starting clan title change test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== CLAN TITLE CHANGE TEST STARTED ===");

    auto bots = this->get_test_bots();

    // Verify initial clan setup: bot0=master, bot1=deputy, bot2=mate
    fb::logger::debug("Verifying initial clan setup - Master: {}, Deputy: {}, Mate: {}", bots[0]->name(), bots[1]->name(), bots[2]->name());

    // Step 1: Bot2 (mate) tries to change clan title - should fail
    fb::logger::debug("Step 1: {} (mate) attempting to change clan title - should fail", bots[2]->name());

    if (co_await bots[2]->change_clan_title("MateTitle", DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan title change test failed: {} (mate) should not be able to change clan title", bots[2]->name());
        bots[0]->chat("=== CLAN TITLE CHANGE TEST FAILED: Mate should not be able to change title ===");
        co_return false;
    }
    fb::logger::debug("Step 1 passed: {} (mate) correctly failed to change clan title", bots[2]->name());

    // Step 2: Bot1 (deputy) tries to change clan title - should fail
    fb::logger::debug("Step 2: {} (deputy) attempting to change clan title - should fail", bots[1]->name());

    if (co_await bots[1]->change_clan_title("DeputyTitle", DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan title change test failed: {} (deputy) should not be able to change clan title", bots[1]->name());
        bots[0]->chat("=== CLAN TITLE CHANGE TEST FAILED: Deputy should not be able to change title ===");
        co_return false;
    }
    fb::logger::debug("Step 2 passed: {} (deputy) correctly failed to change clan title", bots[1]->name());

    // Step 3: Bot0 (master) changes clan title - should succeed
    fb::logger::debug("Step 3: {} (master) changing clan title - should succeed", bots[0]->name());

    if (!co_await bots[0]->change_clan_title("MasterTitle", DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Clan title change test failed: {} (master) should be able to change clan title", bots[0]->name());
        bots[0]->chat("=== CLAN TITLE CHANGE TEST FAILED: Master should be able to change title ===");
        co_return false;
    }
    fb::logger::debug("Step 3 passed: {} (master) successfully changed clan title", bots[0]->name());

    fb::logger::debug("Clan title change test completed successfully");
    bots[0]->chat("=== CLAN TITLE CHANGE TEST COMPLETED SUCCESSFULLY ===");
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
        co_return co_await this->test_clan_role();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_clan_title_change();
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