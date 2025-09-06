#ifndef __BOT_INTEGRATION_WORLDMAP_TEST_H__
#define __BOT_INTEGRATION_WORLDMAP_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/game/protocol.h>
#include <vector>
#include <memory>

namespace fb::bot::integration {

class worldmap_test : public bot_integration_test
{
public:
    worldmap_test(game_bot_controller& controller);
    std::string name() const override final;

private:
    async::task<bool> worldmap_scenario();

protected:
    generator<scenario_t> on_generate_scenario() override final;
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_WORLDMAP_TEST_H__