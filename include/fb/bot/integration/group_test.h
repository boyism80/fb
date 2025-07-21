#ifndef __BOT_INTEGRATION_GROUP_TEST_H__
#define __BOT_INTEGRATION_GROUP_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <stdexcept>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

/**
 * @brief      Group integration test implementation.
 *
 *             Tests bot group functionality and coordination between multiple bots.
 */
class group_test : public bot_integration_test
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
     * @brief      Constructs a new group test with controller reference.
     *
     *             Initializes the group test with 6 bots for group functionality testing.
     *
     * @param[in]  controller  Reference to the parent game bot controller.
     */
    group_test(game_bot_controller& controller);

    /**
     * @brief      Gets the test name.
     *
     * @return     "Group Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Tests group experience sharing mechanics.
     *
     *             This scenario verifies that experience gained in a group is properly
     *             shared and reduced compared to solo play. It spawns monsters and
     *             compares experience gains between solo and group attacks.
     *
     * @return     A task that completes with true if group exp sharing works correctly.
     *
     * @note       Tests the core group mechanics by comparing solo vs group experience gains
     */
    async::task<bool> scenario_1();

    /**
     * @brief      Tests group management functionality.
     *
     *             This scenario tests various group management operations including
     *             inviting members, voluntary leaving, kicking members, and permission
     *             validation for non-master members.
     *
     * @return     A task that completes with true if all group management operations work correctly.
     *
     * @note       Tests invite, leave, kick operations and permission validation
     */
    async::task<bool> scenario_2();
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_GROUP_TEST_H__