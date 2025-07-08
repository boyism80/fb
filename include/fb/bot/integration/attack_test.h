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
     * @brief      Initializes the attack test and spawns required bots.
     *
     *             Spawns 1 bot for attack testing and waits for it to connect.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     *
     * @return     A task that completes when initialization is finished.
     */
    async::task<void> initialize(game_bot_controller& controller);

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
    std::string name() const override final
    {
        return "Attack Test";
    }

    /**
     * @brief      Called when a bot receives an object ID response.
     *
     *             Checks if all bots are ready and starts the test if conditions are met.
     *
     * @param[in]  bot       The bot that received the object ID.
     * @param[in]  response  The object ID response containing the new ID.
     *
     * @return     An async task that completes when hook processing is finished.
     */
    async::task<void> on_hook_sequence(fb::bot::game_bot& bot, const fb::protocol::game::response::id& response);

    /**
     * @brief      Resets the test state to idle.
     */
    void reset() override final;

    /**
     * @brief      Checks if the test is ready to start execution.
     *
     * @return     True if the test is ready to start, false otherwise.
     */
    bool is_ready() const override final;

    /**
     * @brief      Called when a bot connects to the test.
     *
     * @param[in]  bot  Shared pointer to the connected bot.
     */
    void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) override final;
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_ATTACK_TEST_H__