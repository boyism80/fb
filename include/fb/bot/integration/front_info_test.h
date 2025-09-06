#ifndef __FRONT_INFO_TEST_H__
#define __FRONT_INFO_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

class front_info_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;

public:
    front_info_test(game_bot_controller& controller);
    ~front_info_test() = default;
    std::string name() const override final;

private:
    async::task<bool> test_front_info();
};

} // namespace fb::bot::integration

#endif // __FRONT_INFO_TEST_H__