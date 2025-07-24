#ifndef __EMOTION_TEST_H__
#define __EMOTION_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

/**
 * @brief      Integration test for emotion functionality.
 *
 *             This test class focuses on testing emotion-related operations
 *             including emotion packet handling and action responses.
 *             It validates that emotion requests are properly processed
 *             and generate appropriate action responses.
 */
class emotion_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;

public:
    /**
     * @brief      Constructs a new emotion test instance.
     *
     * @param      controller  Reference to the game bot controller.
     */
    emotion_test(game_bot_controller& controller);

    /**
     * @brief      Destroys the emotion test instance.
     */
    ~emotion_test() = default;

    /**
     * @brief      Gets the test name.
     *
     * @return     "Emotion Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Tests emotion packet handler.
     *
     *             This function tests the emotion packet handler by sending
     *             emotion requests with different values and verifying that
     *             appropriate action responses are received.
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_emotion();
};

} // namespace fb::bot::integration

#endif // __EMOTION_TEST_H__