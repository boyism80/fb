#include <fb/bot/integration/clan_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/logger.h>
#include <format>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

clan_test::clan_test(game_bot_controller& controller) :
    bot_integration_test(controller, 4)
{ }

async::task<void> clan_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);
    co_await super::arrange_bots_in_line_formation();
    co_return;
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

async::task<bool> clan_test::run_script_step(const std::shared_ptr<game_bot>&        bot,
                                             std::string_view                        step,
                                             std::initializer_list<std::string_view> extra_args)
{
    auto cmd         = std::format("/스크립트 {} run {}", SCRIPT_FILE, step);
    auto step_str    = std::string(step);
    auto pass_prefix = std::format("CL:PASS:{}", step_str);
    auto fail_prefix = std::format("CL:FAIL:{}", step_str);

    for (auto arg : extra_args)
        cmd.append(std::format(" {}", arg));

    try
    {
        auto&& resp = co_await bot->request<game_resp::chat>(
            game_reqs::chat(false, cmd),
            [pass_prefix, fail_prefix](auto& response) -> bool {
                return response.text.find(pass_prefix) != std::string::npos ||
                       response.text.find(fail_prefix) != std::string::npos;
            },
            DEFAULT_TIMEOUT);

        if (resp.text.find(pass_prefix) == std::string::npos)
        {
            fb::logger::fatal("Clan script step failed ({}): {}", step_str, resp.text);
            co_return false;
        }

        co_return true;
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Clan script step error ({}): {}", step_str, e.what());
        co_return false;
    }
}

async::task<bool> clan_test::test_clan_creation()
{
    auto bots = this->get_test_bots();
    bots[0]->chat("=== CLAN CREATION TEST STARTED ===");

    if (!co_await this->run_script_step(bots[0], "create_clan"))
        co_return false;

    bots[0]->chat("=== CLAN CREATION TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_title()
{
    auto bots = this->get_test_bots();
    bots[0]->chat("=== CLAN TITLE TEST STARTED ===");

    if (!co_await this->run_script_step(bots[0], "set_title"))
        co_return false;

    bots[0]->chat("=== CLAN TITLE TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_invite()
{
    auto bots = this->get_test_bots();
    bots[0]->chat("=== CLAN INVITE TEST STARTED ===");

    if (!co_await this->run_script_step(bots[0], "invite", {bots[1]->name()}))
        co_return false;

    if (!co_await this->run_script_step(bots[1], "verify_member", {bots[0]->name()}))
        co_return false;

    bots[0]->chat("=== CLAN INVITE TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_role()
{
    auto bots = this->get_test_bots();
    bots[0]->chat("=== CLAN ROLE TEST STARTED ===");

    if (!co_await this->run_script_step(bots[1], "invite_fail", {bots[2]->name()}))
        co_return false;

    if (!co_await this->run_script_step(bots[0], "promote_deputy", {bots[1]->name()}))
        co_return false;

    if (!co_await this->run_script_step(bots[1], "invite", {bots[2]->name()}))
        co_return false;

    if (!co_await this->run_script_step(bots[1], "change_role_fail", {bots[2]->name(), "3"}))
        co_return false;

    if (!co_await this->run_script_step(bots[1], "change_role_fail", {bots[3]->name(), "0"}))
        co_return false;

    bots[0]->chat("=== CLAN ROLE TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_title_change()
{
    auto bots = this->get_test_bots();
    bots[0]->chat("=== CLAN TITLE CHANGE TEST STARTED ===");

    if (!co_await this->run_script_step(bots[2], "change_title_fail", {"MateTitle"}))
        co_return false;

    if (!co_await this->run_script_step(bots[1], "change_title_fail", {"DeputyTitle"}))
        co_return false;

    if (!co_await this->run_script_step(bots[0], "change_title", {"MasterTitle"}))
        co_return false;

    bots[0]->chat("=== CLAN TITLE CHANGE TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> clan_test::test_clan_disbanding()
{
    auto bots = this->get_test_bots();
    bots[0]->chat("=== CLAN DISBANDING TEST STARTED ===");

    if (!co_await this->run_script_step(bots[2], "kick_fail", {bots[1]->name()}))
        co_return false;

    if (!co_await this->run_script_step(bots[2], "leave"))
        co_return false;

    if (!co_await this->run_script_step(bots[0], "kick", {bots[1]->name()}))
        co_return false;

    if (!co_await this->run_script_step(bots[0], "destroy_clan"))
        co_return false;

    bots[0]->chat("=== CLAN DISBANDING TEST COMPLETED SUCCESSFULLY ===");
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
