#ifndef __BOT_INTEGRATION_ATTACK_TEST_H__
#define __BOT_INTEGRATION_ATTACK_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>
#include <stdexcept>

namespace fb::bot::integration {

class attack_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

private:
    std::optional<uint32_t> _target_oid;
    bool                    _done = false;

public:
    attack_test(game_bot_controller& controller);
    std::string name() const override final;

private:
    async::task<void> on_hook_die(game_bot& bot, const fb::protocol::game::response::die& response);

private:
    async::task<bool> attack_scenario_1();
    async::task<bool> attack_scenario_2();

protected:
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_ATTACK_TEST_H__