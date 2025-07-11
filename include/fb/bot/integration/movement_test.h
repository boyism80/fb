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
     * @brief      Executes the movement test.
     *
     *             Moves the last bot downward by MOVEMENT_STEPS positions to test movement functionality.
     *             This method is called only when all bots are ready (have non-zero oid and valid position).
     *
     * @return     A task that completes when movement test finishes, returning true on success.
     */
    async::task<bool> execute() override final;

    /**
     * @brief      Gets the test name.
     *
     * @return     "Movement Test" as the identifier.
     */
    std::string name() const override final;

    /**
     * @brief      Resets the test state to idle.
     */
    void reset() override final;
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_MOVEMENT_TEST_H__