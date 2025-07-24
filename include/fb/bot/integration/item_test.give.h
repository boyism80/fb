#ifndef __ITEM_TEST_GIVE_H__
#define __ITEM_TEST_GIVE_H__

#include <fb/bot/integration/test_case.h>
#include <stdexcept>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

/**
 * @brief      Integration test for item give functionality.
 *
 *             This test class focuses on testing item give operations
 *             including giving items to other characters, giving items
 *             to mobs, and various failure scenarios like inventory full
 *             and non-tradeable items.
 */
class item_test_give : public bot_integration_test
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
     * @brief      Constructs a new item test give instance.
     *
     * @param      controller  Reference to the game bot controller.
     */
    item_test_give(game_bot_controller& controller);

    /**
     * @brief      Destroys the item test give instance.
     */
    ~item_test_give() = default;

    /**
     * @brief      Gets the test name.
     *
     * @return     "Item Test Give" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Tests giving non-tradeable item (should fail).
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_give_non_tradeable_item();

    /**
     * @brief      Tests giving tradeable item (should succeed).
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_give_tradeable_item();

    /**
     * @brief      Tests giving item when target inventory is full (should fail).
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_give_item_inventory_full();

    /**
     * @brief      Tests giving maximum money amount (should succeed).
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_give_max_money();

    /**
     * @brief      Tests giving item and money to mob then killing it.
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_give_to_mob_and_kill();
};

} // namespace fb::bot::integration

#endif // __ITEM_TEST_GIVE_H__