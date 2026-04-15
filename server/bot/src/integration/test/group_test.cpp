#include <fb/bot/integration/group_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

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
    fb::logger::debug("Bot {} starting group scenario 1 test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== GROUP SCENARIO 1 TEST STARTED ===");

    auto bots   = this->get_test_bots();
    auto caster = bots.front();

    fb::logger::debug("Learning spell 뢰진주 and setting up MP");
    auto spell_slot = co_await caster->learn_spell("뢰진주", DEFAULT_TIMEOUT);
    co_await caster->change_mp(1000, DEFAULT_TIMEOUT);

    fb::logger::debug("Spawning monster 다람쥐 for solo test");
    auto mob_info1 =
        co_await caster->spawn_monster("다람쥐", caster->position().x, caster->position().y + 1, DEFAULT_TIMEOUT);

    auto before_exp = caster->exp();
    fb::logger::debug("Solo attack - Before exp: {}", before_exp);
    auto&& resp1 = co_await caster->request<game_resp::update_internal>(
        game_reqs::spell_cast(SPELL_TYPE::TARGET, spell_slot, "", mob_info1.oid, mob_info1.position),
        [before_exp](auto& resp) -> bool {
            if (ENUM_IN(resp.level, UPDATE_STATE_LEVEL::EXP_MONEY) == false)
                return false;

            return resp.ch_exp != before_exp;
        },
        DEFAULT_TIMEOUT);

    auto diff_exp = resp1.ch_exp - before_exp;
    fb::logger::debug("Solo attack - Diff exp: {}", diff_exp);

    fb::logger::debug("Forming group for group exp test");
    before_exp = caster->exp();
    co_await this->form_group();
    fb::logger::debug("Group formed successfully");

    fb::logger::debug("Setting up MP and spawning monster for group test");
    co_await caster->change_mp(1000, DEFAULT_TIMEOUT);
    auto mob_info2 =
        co_await caster->spawn_monster("다람쥐", caster->position().x, caster->position().y + 1, DEFAULT_TIMEOUT);
    co_await this->sleep(DEFAULT_INTERVAL);

    fb::logger::debug("Group attack - Before exp: {}", before_exp);
    auto&& resp2 = co_await caster->request<game_resp::update_internal>(
        game_reqs::spell_cast(SPELL_TYPE::TARGET, spell_slot, "", mob_info2.oid, mob_info2.position),
        [before_exp](auto& resp) -> bool {
            if (ENUM_IN(resp.level, UPDATE_STATE_LEVEL::EXP_MONEY) == false)
                return false;

            return resp.ch_exp != before_exp;
        },
        DEFAULT_TIMEOUT);
    auto diff_exp2 = resp2.ch_exp - before_exp;
    fb::logger::debug("Group attack - Diff exp: {}", diff_exp2);

    fb::logger::debug("Comparing exp gains - Solo: {}, Group: {}", diff_exp, diff_exp2);
    if (diff_exp2 >= diff_exp)
    {
        fb::logger::fatal("Group exp gain ({}) should be less than solo exp gain ({})", diff_exp2, diff_exp);
        co_return false;
    }

    fb::logger::debug("Group exp sharing test passed - group exp is correctly reduced");
    caster->chat("=== GROUP SCENARIO 1 TEST COMPLETED SUCCESSFULLY ===");
    co_return true;
}

async::task<bool> group_test::scenario_2()
{
    fb::logger::debug("Bot {} starting group scenario 2 test", this->get_test_bots()[0]->oid());
    this->get_test_bots()[0]->chat("=== GROUP SCENARIO 2 TEST STARTED ===");

    auto bots = this->get_test_bots();

    // Invite first member
    fb::logger::debug("Inviting {} to group", bots[1]->name());
    if (co_await bots[0]->invite_group(bots[1], DEFAULT_TIMEOUT) == false)
    {
        fb::logger::fatal("Failed to invite {} to group", bots[1]->name());
        co_return false;
    }
    fb::logger::debug("Successfully invited {} to group", bots[1]->name());

    // Invite second member
    fb::logger::debug("Inviting {} to group", bots[2]->name());
    if (co_await bots[0]->invite_group(bots[2], DEFAULT_TIMEOUT) == false)
    {
        fb::logger::fatal("Failed to invite {} to group", bots[2]->name());
        co_return false;
    }
    fb::logger::debug("Successfully invited {} to group", bots[2]->name());

    // First member leaves voluntarily
    fb::logger::debug("{} leaving group voluntarily", bots[1]->name());
    if (co_await bots[1]->leave_group(DEFAULT_TIMEOUT) == false)
    {
        fb::logger::fatal("Failed to leave group for {}", bots[1]->name());
        co_return false;
    }
    fb::logger::debug("{} successfully left group", bots[1]->name());

    // Group master kicks second member
    fb::logger::debug("Group master {} kicking {}", bots[0]->name(), bots[2]->name());
    if (co_await bots[0]->kick_group(bots[2], DEFAULT_TIMEOUT) == false)
    {
        fb::logger::fatal("Failed to kick {} from group", bots[2]->name());
        co_return false;
    }
    fb::logger::debug("Successfully kicked {} from group", bots[2]->name());

    // Re-invite first member
    fb::logger::debug("Re-inviting {} to group", bots[1]->name());
    if (co_await bots[0]->invite_group(bots[1], DEFAULT_TIMEOUT) == false)
    {
        fb::logger::fatal("Failed to re-invite {} to group", bots[1]->name());
        co_return false;
    }
    fb::logger::debug("Successfully re-invited {} to group", bots[1]->name());

    // Test that non-master cannot invite (should fail)
    fb::logger::debug("Testing that non-master {} cannot invite {}", bots[1]->name(), bots[2]->name());
    if (co_await bots[1]->invite_group(bots[2], DEFAULT_TIMEOUT))
    {
        fb::logger::fatal("Non-master {} should not be able to invite {} to group", bots[1]->name(), bots[2]->name());
        co_return false;
    }
    fb::logger::debug("Correctly prevented non-master from inviting to group");

    bots[0]->chat("=== GROUP SCENARIO 2 TEST COMPLETED SUCCESSFULLY ===");
    fb::logger::debug("Group scenario 2 test completed successfully");
    co_return true;
}

fb::generator<bot_integration_test::scenario_t> group_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->scenario_1();
    };

    co_yield [this]() -> async::task<bool> {
        co_return co_await this->scenario_2();
    };
}

std::string group_test::name() const
{
    return "Group Test";
}