#include <fb/bot/integration/clan_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/bot/integration/dialog_bot.h>
#include <fb/bot/integration/dialog_ext_bot.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

clan_test::clan_test(game_bot_controller& controller) :
    bot_integration_test(controller, 2)
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
    auto  bots = this->get_test_bots();
    auto& bot  = bots.front();

    auto npc = this->controller.container.model.npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        fb::logger::fatal("Clan test failed: NPC not found");
        co_return false;
    }

    // Click on NPC to open menu dialog
    std::ignore = co_await bot->request<fb::bot::integration::dialog_bot>(
        fb::protocol::game::request::click(1),
        [&npc](auto& resp) {
            return resp.type == fb::bot::integration::dialog_type::menu && resp.look == npc->look;
        },
        DEFAULT_TIMEOUT);

    // Select 1st menu and receive input dialog
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

    bot->send(fb::protocol::game::request::dialog(fb::protocol::game::request::dialog::INTERACTION::NORMAL,
                                                  1,
                                                  "",
                                                  0,
                                                  0,
                                                  "",
                                                  DIALOG_RESULT::PREV));

    auto&& resp =
        co_await bot->request<fb::protocol::game::response::internal_info>(fb::protocol::game::request::self_info(),
                                                                           DEFAULT_TIMEOUT);

    if (resp.clan_name != bot->name())
    {
        fb::logger::fatal("Clan test failed: Clan name mismatch");
        co_return false;
    }

    co_return true;
}

async::task<bool> clan_test::test_clan_member_management()
{
    // TODO: Implement clan member management test
    // - Add members to clan
    // - Remove members from clan
    // - Test clan leave functionality
    // - Test clan kick functionality

    fb::logger::debug("Clan member management test - not implemented yet");
    co_await this->sleep(1s);
    co_return true;
}

async::task<bool> clan_test::test_clan_communication()
{
    // TODO: Implement clan communication test
    // - Test clan chat functionality
    // - Test clan coordination features
    // - Verify communication between clan members

    fb::logger::debug("Clan communication test - not implemented yet");
    co_await this->sleep(1s);
    co_return true;
}

async::task<bool> clan_test::test_clan_disbanding()
{
    // TODO: Implement clan disbanding test
    // - Test clan disbanding scenarios
    // - Verify cleanup when clans are dissolved
    // - Test member state after disbanding

    fb::logger::debug("Clan disbanding test - not implemented yet");
    co_await this->sleep(1s);
    co_return true;
}

fb::generator<bot_integration_test::scenario_t> clan_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_clan_creation();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_clan_member_management();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_clan_communication();
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