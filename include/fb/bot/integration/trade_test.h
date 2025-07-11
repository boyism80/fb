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
    async::task<bool> execute() override;
    void              reset() override;
    std::string       name() const override;

private:
    // Test scenarios
    async::task<bool> test_scenario_1(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);
    async::task<bool> test_scenario_2(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);
    async::task<bool> test_scenario_3(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);
    async::task<bool> test_scenario_4(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);

private:
    // Helper functions
    async::task<void> reset_bot_state(std::shared_ptr<game_bot>& bot);
    bool              has_item(const std::shared_ptr<game_bot>& bot, const std::string& name);
    uint16_t          get_item_count(const std::shared_ptr<game_bot>& bot, const std::string& name);
};

} // namespace fb::bot::integration

#endif //__BOT_INTEGRATION_TRADE_TEST_H__