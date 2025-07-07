#ifndef __BOT_INTEGRATION_TRADE_TEST_H__
#define __BOT_INTEGRATION_TRADE_TEST_H__

#include <fb/bot/integration/test_case.h>

namespace fb::bot::integration {

class trade_test : public bot_integration_test
{
public:
    trade_test(game_bot_controller& controller);
    ~trade_test() = default;

public:
    async::task<void> initialize(game_bot_controller& controller) override;
    async::task<bool> execute() override;
    void              reset() override;
    std::string       name() const override
    {
        return "Trade Test";
    }

    /**
     * @brief      Checks if all spawned bots are ready for the trade test.
     *
     * @return     True if all bots are ready, false otherwise.
     */
    bool is_ready() const override final;

    /**
     * @brief      Called when a bot connects.
     *
     * @param[in]  bot  The connected bot.
     */
    void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) override final;

    /**
     * @brief      Called when a bot receives an object ID response.
     *
     * @param[in]  bot       The bot that received the response.
     * @param[in]  response  The object ID response.
     * @return     An async task.
     */
    async::task<void> on_hook_sequence(fb::bot::game_bot& bot, const fb::protocol::game::response::id& response);

    /**
     * @brief      Called when a bot receives a position response.
     *
     * @param[in]  bot       The bot that received the response.
     * @param[in]  response  The position response.
     * @return     An async task.
     */
    async::task<void> on_hook_position(fb::bot::game_bot& bot, const fb::protocol::game::response::position& response);

private:
    // Test scenarios
    async::task<bool> test_scenario_1(std::shared_ptr<game_bot>& bot1,
                                      std::shared_ptr<game_bot>& bot2,
                                      const fb::model::timespan& timeout);
    async::task<bool> test_scenario_2(std::shared_ptr<game_bot>& bot1,
                                      std::shared_ptr<game_bot>& bot2,
                                      const fb::model::timespan& timeout);
    async::task<bool> test_scenario_3(std::shared_ptr<game_bot>& bot1,
                                      std::shared_ptr<game_bot>& bot2,
                                      const fb::model::timespan& timeout);
    async::task<bool> test_scenario_4(std::shared_ptr<game_bot>& bot1,
                                      std::shared_ptr<game_bot>& bot2,
                                      const fb::model::timespan& timeout);

private:
    // Helper functions
    async::task<void> reset_bot_state(std::shared_ptr<game_bot>& bot, const fb::model::timespan& timeout);
    bool              has_item(const std::shared_ptr<game_bot>& bot, const std::string& name);
    uint16_t          get_item_count(const std::shared_ptr<game_bot>& bot, const std::string& name);
};

} // namespace fb::bot::integration

#endif //__BOT_INTEGRATION_TRADE_TEST_H__