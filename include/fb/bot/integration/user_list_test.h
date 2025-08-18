#ifndef __USER_LIST_TEST_H__
#define __USER_LIST_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

class user_list_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;

public:
    user_list_test(game_bot_controller& controller);
    ~user_list_test() = default;
    std::string name() const override final;

private:
    async::task<bool> test_scenario_1();
};

} // namespace fb::bot::integration

#endif // __USER_LIST_TEST_H__