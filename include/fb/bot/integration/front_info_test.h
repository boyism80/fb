#ifndef __FRONT_INFO_TEST_H__
#define __FRONT_INFO_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

/**
 * @brief      Integration test for front_info functionality.
 *
 *             This test class focuses on testing front_info packet handling
 *             including object detection in front of the character and
 *             message response validation.
 */
class front_info_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;

public:
    /**
     * @brief      Constructs a new front_info test instance.
     *
     * @param      controller  Reference to the game bot controller.
     */
    front_info_test(game_bot_controller& controller);

    /**
     * @brief      Destroys the front_info test instance.
     */
    ~front_info_test() = default;

    /**
     * @brief      Gets the test name.
     *
     * @return     "Front Info Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Tests front_info packet handler.
     *
     *             This function tests the front_info packet handler by positioning
     *             bots, creating items and money, dropping them, and verifying
     *             that the front_info response contains the expected messages.
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_front_info();
};

} // namespace fb::bot::integration

#endif // __FRONT_INFO_TEST_H__