#ifndef __CHAT_INTERACTION_TEST_H__
#define __CHAT_INTERACTION_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>
#include <fb/bot/integration/dialog_bot.h>
#include <fb/bot/integration/dialog_ext_bot.h>

namespace fb::bot::integration {

class chat_interaction_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_parallel_scenario_started(uint32_t id) override final;
    async::task<void>     on_parallel_scenario_finished(uint32_t id) override final;

public:
    chat_interaction_test(game_bot_controller& controller);
    ~chat_interaction_test() = default;
    std::string name() const override final;

private:
    async::task<bool> test_scenario_1(int index);
    async::task<bool> test_scenario_2(int index);
    async::task<bool> test_scenario_3(int index);
    async::task<bool> test_scenario_4(int index);

private:
    async::task<bool> parallel_scenario_1();
};

} // namespace fb::bot::integration

#endif // __CHAT_INTERACTION_TEST_H__