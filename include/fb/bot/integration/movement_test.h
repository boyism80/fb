#ifndef __BOT_INTEGRATION_MOVEMENT_TEST_H__
#define __BOT_INTEGRATION_MOVEMENT_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/game/protocol.h>
#include <vector>
#include <memory>

namespace fb::bot::integration {

/**
 * @brief      Movement integration test implementation.
 *
 *             Tests bot movement by spawning 5 bots and moving the last bot
 *             downward by 5 positions with 100ms intervals.
 */
class movement_test : public bot_integration_test
{
private:
    static constexpr int MOVEMENT_STEPS = 5;

public:
    /**
     * @brief      Constructs a new movement test with controller reference.
     *
     *             Initializes the movement test with 5 bots and registers common hooks.
     *
     * @param[in]  controller  Reference to the parent game bot controller.
     */
    movement_test(game_bot_controller& controller);

    /**
     * @brief      Gets the test name.
     *
     * @return     "Movement Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Moves the last bot downward by MOVEMENT_STEPS positions.
     *
     * @return     A task that completes when the bot is moved.
     */
    async::task<bool> move_bot_downward();

protected:
    /**
     * @brief      Generates a scenario function for the test.
     *
     * @return     A generator that yields scenario functions.
     */
    generator<scenario_t> on_generate_scenario() override final;
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_MOVEMENT_TEST_H__