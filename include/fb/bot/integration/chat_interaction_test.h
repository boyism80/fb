#ifndef __CHAT_INTERACTION_TEST_H__
#define __CHAT_INTERACTION_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

/**
 * @brief      Integration test for chat interaction functionality.
 *
 *             This test class focuses on testing chat-based interactions
 *             including NPC creation/removal, item trading, money management,
 *             and various chat commands. It validates that chat commands
 *             are properly processed and generate appropriate responses.
 */
class chat_interaction_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;

public:
    /**
     * @brief      Constructs a new chat interaction test instance.
     *
     * @param      controller  Reference to the game bot controller.
     */
    chat_interaction_test(game_bot_controller& controller);

    /**
     * @brief      Destroys the chat interaction test instance.
     */
    ~chat_interaction_test() = default;

    /**
     * @brief      Gets the test name.
     *
     * @return     "Chat Interaction Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Tests chat interaction scenario 1.
     *
     *             This function tests various chat-based interactions including
     *             NPC creation/removal, item trading, money management, and
     *             chat commands. It validates that all responses are correct
     *             and game state changes appropriately.
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_scenario_1();
};

} // namespace fb::bot::integration

#endif // __CHAT_INTERACTION_TEST_H__