#ifndef __EMOTION_TEST_H__
#define __EMOTION_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

class emotion_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;

public:
    emotion_test(game_bot_controller& controller);
    ~emotion_test() = default;
    std::string name() const override final;

private:
    async::task<bool> test_emotion();
};

} // namespace fb::bot::integration

#endif // __EMOTION_TEST_H__