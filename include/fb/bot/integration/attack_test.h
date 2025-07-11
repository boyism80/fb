#ifndef __BOT_INTEGRATION_ATTACK_TEST_H__
#define __BOT_INTEGRATION_ATTACK_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/game/protocol.h>
#include <vector>
#include <memory>

namespace fb::bot::integration {

/**
 * @brief      Attack integration test implementation.
 *
 *             Tests bot attack functionality by spawning a monster and having
 *             the bot attack it until the monster is defeated.
 */
class attack_test : public bot_integration_test
{
public:
    /**
     * @brief      Constructs a new attack test with controller reference.
     *
     *             Initializes the attack test and registers hooks for specific
     *             protocol types to enable event-driven test execution.
     *
     * @param[in]  controller  Reference to the parent game bot controller.
     */
    attack_test(game_bot_controller& controller);

    /**
     * @brief      Executes the attack test.
     *
     *             Spawns a monster and has the bot attack it until defeated.
     *             This method is called only when all bots are ready (have non-zero oid).
     *
     * @return     A task that completes when attack test finishes, returning true on success.
     */
    async::task<bool> execute();

    /**
     * @brief      Gets the test name.
     *
     * @return     "Attack Test" as the identifier.
     */
    std::string name() const override final;

    /**
     * @brief      Resets the test state to idle.
     */
    void reset() override final;
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_ATTACK_TEST_H__