#ifndef __BOT_INTEGRATION_WORLDMAP_TEST_H__
#define __BOT_INTEGRATION_WORLDMAP_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/game/protocol.h>
#include <vector>
#include <memory>

namespace fb::bot::integration {

/**
 * @brief      Worldmap integration test implementation.
 *
 *             Tests worldmap functionality by spawning bots and performing
 *             worldmap-related operations such as map transitions and
 *             worldmap data retrieval.
 */
class worldmap_test : public bot_integration_test
{
public:
    /**
     * @brief      Constructs a new worldmap test with controller reference.
     *
     *             Initializes the worldmap test with DEFAULT_BOT_COUNT bots
     *             and registers common hooks.
     *
     * @param[in]  controller  Reference to the parent game bot controller.
     */
    worldmap_test(game_bot_controller& controller);

    /**
     * @brief      Gets the test name.
     *
     * @return     "Worldmap Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Performs basic worldmap scenario testing.
     *
     *             This is a placeholder for future worldmap-specific test
     *             implementations such as map transitions, worldmap data
     *             retrieval, and navigation testing.
     *
     * @return     A task that completes when the worldmap test is finished.
     */
    async::task<bool> worldmap_scenario();

protected:
    /**
     * @brief      Generates a scenario function for the test.
     *
     * @return     A generator that yields scenario functions.
     */
    generator<scenario_t> on_generate_scenario() override final;
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_WORLDMAP_TEST_H__