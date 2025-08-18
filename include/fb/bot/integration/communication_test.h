#ifndef __BOT_INTEGRATION_COMMUNICATION_TEST_H__
#define __BOT_INTEGRATION_COMMUNICATION_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

class communication_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

public:
    communication_test(game_bot_controller& controller);
    ~communication_test() override = default;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;

public:
    std::string name() const override final;

private:
    async::task<bool> test_normal_chat();
    async::task<bool> test_shout_chat();
    async::task<bool> test_whisper();
    async::task<bool> test_whisper_block();

private:
    static constexpr int REQUIRED_BOTS = 2; ///< Number of bots required for testing
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_COMMUNICATION_TEST_H__