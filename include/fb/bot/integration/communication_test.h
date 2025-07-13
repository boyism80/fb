#ifndef __BOT_INTEGRATION_COMMUNICATION_TEST_H__
#define __BOT_INTEGRATION_COMMUNICATION_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

/**
 * @brief   Communication test class for testing chat, shout, and whisper functionality
 *
 *          This test class verifies the communication features of the game server
 *          including normal chat messages, shout messages, and whisper messages.
 *          It also tests the whisper blocking functionality.
 */
class communication_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

public:
    /**
     * @brief   Constructor for communication test
     *
     * @param[in]  controller  Reference to the game bot controller
     */
    communication_test(game_bot_controller& controller);

    /**
     * @brief   Destructor for communication test
     */
    ~communication_test() override = default;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;

public:
    /**
     * @brief   Get the name of the test
     *
     * @return  "Communication Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief   Test normal chat functionality
     *
     * @return  Task that returns true if test passes, false otherwise
     */
    async::task<bool> test_normal_chat();

    /**
     * @brief   Test shout functionality
     *
     * @return  Task that returns true if test passes, false otherwise
     */
    async::task<bool> test_shout_chat();

    /**
     * @brief   Test whisper functionality
     *
     * @return  Task that returns true if test passes, false otherwise
     */
    async::task<bool> test_whisper();

    /**
     * @brief   Test whisper blocking functionality
     *
     * @return  Task that returns true if test passes, false otherwise
     */
    async::task<bool> test_whisper_block();

private:
    static constexpr int REQUIRED_BOTS = 2; ///< Number of bots required for testing
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_COMMUNICATION_TEST_H__