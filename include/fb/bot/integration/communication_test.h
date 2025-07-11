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

public:
    /**
     * @brief   Execute the communication test scenarios
     *
     * @return  Task that returns true if all tests pass, false otherwise
     */
    async::task<bool> execute() override;

    /**
     * @brief   Get the name of the test
     *
     * @return  "Communication Test" as the identifier.
     */
    std::string name() const override final;

    /**
     * @brief   Reset the test state
     */
    void reset() override;

private:

private:
    /**
     * @brief   Test normal chat functionality
     *
     * @param[in]  bot1     First bot for testing
     * @param[in]  bot2     Second bot for testing
     * @return  Task that returns true if test passes, false otherwise
     */
    async::task<bool> test_normal_chat(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);

    /**
     * @brief   Test shout functionality
     *
     * @param[in]  bot1     First bot for testing
     * @param[in]  bot2     Second bot for testing
     * @return  Task that returns true if test passes, false otherwise
     */
    async::task<bool> test_shout_chat(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);

    /**
     * @brief   Test whisper functionality
     *
     * @param[in]  bot1     First bot for testing
     * @param[in]  bot2     Second bot for testing
     * @return  Task that returns true if test passes, false otherwise
     */
    async::task<bool> test_whisper(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);

    /**
     * @brief   Test whisper blocking functionality
     *
     * @param[in]  bot1     First bot for testing
     * @param[in]  bot2     Second bot for testing
     * @return  Task that returns true if test passes, false otherwise
     */
    async::task<bool> test_whisper_block(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);

    /**
     * @brief   Reset bot state for testing
     *
     * @param[in]  bot      Bot to reset
     * @return  Task that completes when reset is finished
     */
    async::task<void> reset_bot_state(std::shared_ptr<game_bot>& bot);

private:
    static constexpr int REQUIRED_BOTS = 2; ///< Number of bots required for testing
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_COMMUNICATION_TEST_H__