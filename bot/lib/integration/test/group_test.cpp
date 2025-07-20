#include <fb/bot/integration/group_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

group_test::group_test(game_bot_controller& controller) :
    bot_integration_test(controller, 6)
{ }

async::task<void> group_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);

    auto bots = this->get_test_bots();
    fb::logger::debug("Starting group test with {} bots", bots.size());

    if (bots.empty())
        throw std::runtime_error("No bots available for group test");

    co_await super::arrange_bots_in_line_formation();
}

async::task<void> group_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> group_test::on_scenario_finished(uint32_t scenario_index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots.front();
    co_await bot->clear_all_drop_items(DEFAULT_TIMEOUT);
    co_await this->cleanup_group();
}

async::task<bool> group_test::scenario_1()
{
    auto bots   = this->get_test_bots();
    auto caster = bots.front();
    co_await caster->learn_spell("뢰진주", DEFAULT_TIMEOUT);
    co_await caster->change_mp(1000, DEFAULT_TIMEOUT);
    auto mob_info1 =
        co_await caster->spawn_monster("다람쥐", caster->position().x, caster->position().y + 1, DEFAULT_TIMEOUT);

    auto   before_exp = caster->exp();
    auto&& resp1      = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, 0, "", mob_info1.oid, mob_info1.position),
        [before_exp](auto& resp) -> bool {
            if (ENUM_IN(resp.level, STATE_LEVEL::EXP_MONEY) == false)
                return false;

            return resp.ch_exp != before_exp;
        },
        DEFAULT_TIMEOUT);

    auto diff_exp = resp1.ch_exp - before_exp;
    fb::logger::debug("Diff exp: {}", diff_exp);

    before_exp = caster->exp();
    co_await this->form_group();
    co_await caster->change_mp(1000, DEFAULT_TIMEOUT);
    auto mob_info2 =
        co_await caster->spawn_monster("다람쥐", caster->position().x, caster->position().y + 1, DEFAULT_TIMEOUT);
    co_await this->sleep(DEFAULT_INTERVAL);
    auto&& resp2 = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, 0, "", mob_info2.oid, mob_info2.position),
        [before_exp](auto& resp) -> bool {
            if (ENUM_IN(resp.level, STATE_LEVEL::EXP_MONEY) == false)
                return false;

            return resp.ch_exp != before_exp;
        },
        DEFAULT_TIMEOUT);
    auto diff_exp2 = resp2.ch_exp - before_exp;
    fb::logger::debug("Diff exp2: {}", diff_exp2);

    if (diff_exp2 >= diff_exp)
    {
        co_return false;
    }

    co_return true;
}

fb::generator<bot_integration_test::scenario_t> group_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->scenario_1();
    };
}

std::string group_test::name() const
{
    return "Group Test";
}

} // namespace fb::bot::integration