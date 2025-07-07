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
 *             Tests bot movement by checking if all bots have non-zero oid IDs,
 *             then moving the last bot downward by 5 positions with 1-second intervals.
 */
class movement_test : public bot_integration_test
{
private:
    static constexpr int MOVEMENT_STEPS = 5;

public:
    /**
     * @brief      Constructs a new movement test with controller reference.
     *
     *             Initializes the movement test and registers hooks for specific
     *             protocol types to enable event-driven test execution.
     *
     * @param[in]  controller  Reference to the parent game bot controller.
     */
    movement_test(game_bot_controller& controller);

    /**
     * @brief      Initializes the movement test and spawns required bots.
     *
     *             Spawns 1 bot for movement testing and waits for it to connect.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     *
     * @return     A task that completes when initialization is finished.
     */
    async::task<void> initialize(game_bot_controller& controller);

    /**
     * @brief      Executes the movement test.
     *
     *             Moves the bot in a square pattern to test movement functionality.
     *             This method is called only when all bots are ready (have non-zero oid).
     *
     * @return     A task that completes when movement test finishes, returning true on success.
     */
    async::task<bool> execute();

    /**
     * @brief      Gets the test name.
     *
     * @return     "Movement Test" as the identifier.
     */
    std::string name() const override final
    {
        return "Movement Test";
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

    /**
     * @brief      Performs cleanup operations when the test is destroyed.
     */
    void cleanup() override final;
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_MOVEMENT_TEST_H__