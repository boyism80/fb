#ifndef __BOT_INTEGRATION_SWAP_TEST_H__
#define __BOT_INTEGRATION_SWAP_TEST_H__

#include <fb/bot/integration/test_case.h>

namespace fb::bot::integration {

class swap_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

public:
    swap_test(game_bot_controller& controller);

private:
    async::task<void>     on_initialize(game_bot_controller& controller) override;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override;
    async::task<void>     on_parallel_scenario_started(uint32_t id) override;
    async::task<void>     on_parallel_scenario_finished(uint32_t id) override;
    generator<scenario_t> on_generate_scenario() override;
    std::string           name() const override;

private:
    async::task<bool> test_item_swap_1(uint32_t index);
    async::task<bool> test_item_swap_2(uint32_t index);
    async::task<bool> test_item_swap_3(uint32_t index);
    async::task<bool> test_spell_swap_1(uint32_t index);
    async::task<bool> test_spell_swap_2(uint32_t index);
    async::task<bool> test_spell_swap_3(uint32_t index);
};

} // namespace fb::bot::integration

#endif