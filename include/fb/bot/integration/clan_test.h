#ifndef __BOT_INTEGRATION_CLAN_TEST_H__
#define __BOT_INTEGRATION_CLAN_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <stdexcept>
#include <fb/game/protocol.h>
#include <fb/model/model.h>

namespace fb::bot::integration {

class clan_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;
    async::task<void>     on_finished() override final;

public:
    clan_test(game_bot_controller& controller);
    std::string name() const override final;

private:
    async::task<bool> test_clan_creation();
    async::task<bool> test_clan_title();
    async::task<bool> test_clan_invite();
    async::task<bool> invite_to_clan(std::shared_ptr<game_bot> inviter, std::shared_ptr<game_bot> invitee);
    async::task<bool> change_clan_role(std::shared_ptr<game_bot> changer,
                                      std::shared_ptr<game_bot> target,
                                      fb::model::enum_value::CLAN_ROLE role);
    async::task<bool> leave_clan(std::shared_ptr<game_bot> bot);
    async::task<bool> destroy_clan(std::shared_ptr<game_bot> bot);
    async::task<bool> kick_from_clan(std::shared_ptr<game_bot> kicker, std::shared_ptr<game_bot> target);
    async::task<bool> change_clan_title(std::shared_ptr<game_bot> bot, std::string_view title);
    async::task<bool> test_clan_role();
    async::task<bool> test_clan_disbanding();
    async::task<bool> test_clan_title_change();

    uint32_t _clan_npc_oid = 0;
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_CLAN_TEST_H__