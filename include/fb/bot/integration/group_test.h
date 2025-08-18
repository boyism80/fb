#ifndef __BOT_INTEGRATION_GROUP_TEST_H__
#define __BOT_INTEGRATION_GROUP_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <stdexcept>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

class group_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;

public:
    group_test(game_bot_controller& controller);
    std::string name() const override final;

private:
    async::task<bool> scenario_1();
    async::task<bool> scenario_2();
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_GROUP_TEST_H__