#ifndef __BOT_INTEGRATION_ATTACK_TEST_H__
#define __BOT_INTEGRATION_ATTACK_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>
#include <stdexcept>

namespace fb::bot::integration {

/**
 * @brief      Attack integration test implementation.
 *
 *             Tests bot attack functionality by spawning a monster and having
 *             the bot attack it until the monster is defeated.
 */
class attack_test : public bot_integration_test
{
private:
    std::optional<uint32_t> _target_oid;
    bool                    _done = false;

public:
    /**
     * @brief      Constructs a new attack test with controller reference.
     *
     *             Initializes the attack test and registers hooks for specific
     *             protocol types to enable event-driven test execution.
     *
     * @param[in]  controller  Reference to the parent game bot controller.
     */
    attack_test(game_bot_controller& controller);

    /**
     * @brief      Gets the test name.
     *
     * @return     "Attack Test" as the identifier.
     */
    std::string name() const override final;

private:
    async::task<void> on_hook_die(game_bot& bot, const fb::protocol::game::response::die& response);

private:
    async::task<bool> attack_scenario();

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_ATTACK_TEST_H__