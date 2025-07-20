#ifndef __BOT_INTEGRATION_CLAN_TEST_H__
#define __BOT_INTEGRATION_CLAN_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <stdexcept>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

/**
 * @brief      Clan integration test implementation.
 *
 *             Tests bot clan functionality and coordination between multiple bots.
 */
class clan_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;
    async::task<void>     on_parallel_scenario_started(uint32_t id) override final;
    async::task<void>     on_parallel_scenario_finished(uint32_t id) override final;

public:
    /**
     * @brief      Constructs a new clan test with controller reference.
     *
     *             Initializes the clan test with 2 bots for clan functionality testing.
     *
     * @param[in]  controller  Reference to the parent game bot controller.
     */
    clan_test(game_bot_controller& controller);

    /**
     * @brief      Gets the test name.
     *
     * @return     "Clan Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Executes the main clan test scenario.
     *
     *             This method orchestrates the complete clan testing sequence.
     *             Currently contains empty scenarios for future implementation.
     *
     * @return     A task that completes with true if all clan tests passed, false otherwise.
     *
     * @note       This is the primary entry point for clan testing execution
     */
    async::task<bool> scenario_1();

    /**
     * @brief      Tests basic clan creation functionality.
     *
     *             Tests clan creation with multiple bots and verifies
     *             clan membership and coordination.
     *
     * @return     A task that completes when clan creation tests finish.
     */
    async::task<bool> test_clan_creation();

    /**
     * @brief      Tests clan member management functionality.
     *
     *             Tests adding and removing members from clans,
     *             including clan leave and kick scenarios.
     *
     * @return     A task that completes when clan member management tests finish.
     */
    async::task<bool> test_clan_member_management();

    /**
     * @brief      Tests clan communication functionality.
     *
     *             Tests clan chat and coordination features
     *             between clan members.
     *
     * @return     A task that completes when clan communication tests finish.
     */
    async::task<bool> test_clan_communication();

    /**
     * @brief      Tests clan disbanding functionality.
     *
     *             Tests clan disbanding scenarios and cleanup
     *             when clans are dissolved.
     *
     * @return     A task that completes when clan disbanding tests finish.
     */
    async::task<bool> test_clan_disbanding();
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_CLAN_TEST_H__