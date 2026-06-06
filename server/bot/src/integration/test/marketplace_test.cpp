#include <fb/bot/integration/marketplace_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/game_bot.h>
#include <fb/logger.h>
#include <format>

using namespace std::chrono_literals;
using namespace fb::bot::integration;
using fb::bot::game_bot;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

marketplace_test::marketplace_test(game_bot_controller& controller) :
    bot_integration_test(controller, 2)
{ }

std::string marketplace_test::name() const
{
    return "Marketplace Test";
}

generator<bot_integration_test::scenario_t> marketplace_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_marketplace_scenario();
    };
    co_return;
}

async::task<void> marketplace_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);
    co_await super::arrange_bots_in_line_formation();
    co_return;
}

async::task<void> marketplace_test::on_scenario_finished(uint32_t scenario_index)
{
    for (auto& bot : this->get_test_bots())
        co_await this->cleanup_bot(bot);

    co_await this->sleep(1s);
}

async::task<bool> marketplace_test::cleanup_bot(const std::shared_ptr<game_bot>& bot)
{
    bot->chat("/아이템초기화");
    bot->chat("/아이템삭제");
    co_await bot->change_money(0, DEFAULT_TIMEOUT);
    co_return true;
}

async::task<bool> marketplace_test::run_script_step(const std::shared_ptr<game_bot>& bot,
                                                    std::string_view                 role,
                                                    std::string_view                 step)
{
    auto cmd         = std::format("/스크립트 {} {} {}", SCRIPT_FILE, role, step);
    auto step_str    = std::string(step);
    auto pass_prefix = std::format("MP:PASS:{}", step_str);
    auto fail_prefix = std::format("MP:FAIL:{}", step_str);

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
            fb::logger::fatal("Marketplace script step failed ({}): {}", step_str, resp.text);
            co_return false;
        }

        co_return true;
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Marketplace script step error ({}): {}", step_str, e.what());
        co_return false;
    }
}

async::task<bool> marketplace_test::test_marketplace_scenario()
{
    auto bots = this->get_test_bots();
    if (bots.size() < 2)
        co_return false;

    auto& bot_a = bots[0];
    auto& bot_b = bots[1];

    try
    {
        co_await this->cleanup_bot(bot_a);
        co_await this->cleanup_bot(bot_b);

        fb::logger::debug("Marketplace test step 1: verify empty marketplace");
        if (!co_await this->run_script_step(bot_a, "run_a", "verify_empty"))
            co_return false;

        fb::logger::debug("Marketplace test step 2-3: prepare items");
        if (!co_await this->run_script_step(bot_a, "run_a", "prepare_weapon"))
            co_return false;
        if (!co_await this->run_script_step(bot_b, "run_b", "prepare_bundle"))
            co_return false;

        fb::logger::debug("Marketplace test step 4: list weapon with fee failure then success");
        if (!co_await this->run_script_step(bot_a, "run_a", "list_weapon_fee_fail"))
            co_return false;
        if (!co_await this->run_script_step(bot_a, "run_a", "list_weapon_ok"))
            co_return false;

        fb::logger::debug("Marketplace test step 5: verify one weapon listing");
        if (!co_await this->run_script_step(bot_a, "run_a", "verify_weapon_count"))
            co_return false;

        fb::logger::debug("Marketplace test step 6: list bundle item");
        if (!co_await this->run_script_step(bot_b, "run_b", "list_bundle"))
            co_return false;

        fb::logger::debug("Marketplace test step 7: verify two listings");
        if (!co_await this->run_script_step(bot_a, "run_a", "verify_two_listings"))
            co_return false;

        fb::logger::debug("Marketplace test step 8: cancel weapon listing and re-list");
        if (!co_await this->run_script_step(bot_a, "run_a", "cancel_relist"))
            co_return false;

        fb::logger::debug("Marketplace test step 9: purchase renamed weapon");
        if (!co_await this->run_script_step(bot_b, "run_b", "purchase_weapon"))
            co_return false;

        fb::logger::debug("Marketplace test step 10: purchase bundle listing");
        if (!co_await this->run_script_step(bot_a, "run_a", "purchase_bundle"))
            co_return false;

        fb::logger::debug("Marketplace test step 11: verify empty marketplace");
        if (!co_await this->run_script_step(bot_a, "run_a", "verify_empty_final"))
            co_return false;

        bot_a->chat("=== MARKETPLACE TEST COMPLETED SUCCESSFULLY ===");
        co_return true;
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Marketplace test failed: {}", e.what());
        co_return false;
    }
}
