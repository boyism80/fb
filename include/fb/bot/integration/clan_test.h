#ifndef __BOT_INTEGRATION_CLAN_TEST_H__
#define __BOT_INTEGRATION_CLAN_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/bot/game_bot.h>
#include <initializer_list>
#include <string>
#include <string_view>

namespace fb::bot::integration {

class clan_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

public:
    clan_test(game_bot_controller& controller);

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;
    std::string           name() const override final;

private:
    async::task<bool> test_clan_creation();
    async::task<bool> test_clan_title();
    async::task<bool> test_clan_invite();
    async::task<bool> test_clan_role();
    async::task<bool> test_clan_title_change();
    async::task<bool> test_clan_disbanding();

    async::task<bool> run_script_step(const std::shared_ptr<game_bot>&        bot,
                                      std::string_view                        step,
                                      std::initializer_list<std::string_view> extra_args = {});

    static constexpr std::string_view SCRIPT_FILE = "scripts/integration.lua";
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_CLAN_TEST_H__
