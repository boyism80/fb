#ifndef __BOT_INTEGRATION_MARKETPLACE_TEST_H__
#define __BOT_INTEGRATION_MARKETPLACE_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <string>
#include <string_view>

namespace fb::bot {

class game_bot;

} // namespace fb::bot

namespace fb::bot::integration {

class marketplace_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

public:
    marketplace_test(game_bot_controller& controller);

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;
    std::string           name() const override final;

private:
    async::task<bool> test_marketplace_scenario();
    async::task<bool> run_script_step(const std::shared_ptr<game_bot>& bot,
                                      std::string_view                 role,
                                      std::string_view                 step);
    async::task<bool> cleanup_bot(const std::shared_ptr<game_bot>& bot);

    static constexpr std::string_view SCRIPT_FILE = "scripts/integration.lua";
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_MARKETPLACE_TEST_H__
