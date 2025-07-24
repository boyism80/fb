#ifndef __BOT_INTEGRATION_DOOR_TEST_H__
#define __BOT_INTEGRATION_DOOR_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/game/protocol.h>
#include <vector>
#include <memory>

namespace fb::bot::integration {

/**
 * @brief      Door integration test implementation.
 *
 *             Tests door functionality by spawning bots and performing
 *             door-related operations such as door interactions and
 *             door state validation.
 */
class door_test : public bot_integration_test
{
public:
    /**
     * @brief      Constructs a new door test with controller reference.
     *
     *             Initializes the door test with 1 bot and registers common hooks.
     *
     * @param[in]  controller  Reference to the parent game bot controller.
     */
    door_test(game_bot_controller& controller);

    /**
     * @brief      Gets the test name.
     *
     * @return     "Door Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Performs basic door scenario testing.
     *
     *             This is a placeholder for future door-specific test
     *             implementations such as door interactions, door state
     *             validation, and door transition testing.
     *
     * @return     A task that completes when the door test is finished.
     */
    async::task<bool> door_scenario();

protected:
    /**
     * @brief      Generates a scenario function for the test.
     *
     * @return     A generator that yields scenario functions.
     */
    generator<scenario_t> on_generate_scenario() override final;
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_DOOR_TEST_H__