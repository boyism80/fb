#ifndef __BOT_INTEGRATION_MOVEMENT_TEST_H__
#define __BOT_INTEGRATION_MOVEMENT_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/game/protocol.h>
#include <vector>
#include <memory>

namespace fb::bot::integration {

class movement_test : public bot_integration_test
{
private:
    static constexpr int MOVEMENT_STEPS = 5;

public:
    movement_test(game_bot_controller& controller);
    std::string name() const override final;

private:
    async::task<bool> move_bot_downward();

protected:
    generator<scenario_t> on_generate_scenario() override final;
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_MOVEMENT_TEST_H__