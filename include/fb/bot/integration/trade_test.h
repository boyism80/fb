#ifndef __BOT_INTEGRATION_TRADE_TEST_H__
#define __BOT_INTEGRATION_TRADE_TEST_H__

#include <fb/bot/integration/test_case.h>

namespace fb::bot::integration {

class trade_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

public:
    trade_test(game_bot_controller& controller);
    ~trade_test() = default;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;

public:
    std::string name() const override final;

private:
    // Test scenarios
    async::task<bool> test_scenario_1();
    async::task<bool> test_scenario_2();
    async::task<bool> test_scenario_3();
    async::task<bool> test_scenario_4();
    async::task<bool> test_scenario_5();

private:
    // Helper functions
    bool     has_item(const std::shared_ptr<game_bot>& bot, const std::string& name);
    uint16_t get_item_count(const std::shared_ptr<game_bot>& bot, const std::string& name);
};

} // namespace fb::bot::integration

#endif //__BOT_INTEGRATION_TRADE_TEST_H__