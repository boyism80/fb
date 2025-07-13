#ifndef __BOT_INTEGRATION_THROW_TEST_H__
#define __BOT_INTEGRATION_THROW_TEST_H__

#include <fb/bot/integration/test_case.h>

namespace fb::bot::integration {

class throw_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

public:
    throw_test(game_bot_controller& controller);

private:
    async::task<void>     on_initialize(game_bot_controller& controller) override;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override;
    async::task<void>     on_parallel_scenario_started(uint32_t id) override;
    async::task<void>     on_parallel_scenario_finished(uint32_t id) override;
    generator<scenario_t> on_generate_scenario() override;
    std::string           name() const override;

private:
    async::task<bool> test_throw_single_item();
    async::task<bool> test_throw_all_items();
};

} // namespace fb::bot::integration

#endif