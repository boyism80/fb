#ifndef __ITEM_TEST_H__
#define __ITEM_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <stdexcept>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

/**
 * @brief      Integration test for item usage functionality.
 *
 *             This test class focuses on testing item-related operations
 *             including equipment activation, deactivation, and various
 *             item usage scenarios. It validates both successful and
 *             failed equipment operations with appropriate error messages.
 */
class item_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

    /**
     * @brief      Equipment test structure for organizing test data.
     */
    struct equipment_test_data
    {
        std::string item_name;       ///< Name of the item to test
        std::string success_message; ///< Expected success message pattern
        std::string failure_message; ///< Expected failure message for negative tests
        uint8_t     required_level;  ///< Required level for the item
        uint8_t     required_str;    ///< Required strength for the item
        uint8_t     required_dex;    ///< Required dexterity for the item
        uint8_t     required_int;    ///< Required intelligence for the item
    };

    /**
     * @brief      Simple item test data without condition information.
     */
    struct equipment_item_data
    {
        using condition_function = std::function<async::task<void>(fb::bot::game_bot&)>;

        std::string        item_name;       ///< Name of the item to test
        std::string        success_message; ///< Expected success message pattern when equipped
        EQUIPMENT_PARTS    equipment_part;  ///< Equipment part for unequipping
        condition_function condition;       ///< Condition function
        condition_function rollback;        ///< Rollback function
    };

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;
    async::task<void>     on_parallel_scenario_started(uint32_t id) override final;
    async::task<void>     on_parallel_scenario_finished(uint32_t id) override final;

public:
    /**
     * @brief      Constructs a new item test instance.
     *
     * @param      controller  Reference to the game bot controller.
     */
    item_test(game_bot_controller& controller);

    /**
     * @brief      Destroys the item test instance.
     */
    ~item_test() = default;

    /**
     * @brief      Gets the test name.
     *
     * @return     "Item Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Executes all registered test functions.
     *
     * @param      bots  Vector of bot instances to use for testing.
     *
     * @return     An async task that completes with true if all tests passed, false otherwise.
     */
    async::task<bool> execute_test_functions(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots);

    // Test function implementations
    /**
     * @brief      Tests equipment activation with success scenarios.
     *
     *             This function tests the successful equipping of items by characters
     *             with appropriate stats. It ensures that items can be equipped
     *             without any issues when the character meets the required conditions.
     *
     * @param[in]  index    The index of the test bot to use for equipment testing.
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_equipment_success(uint32_t index);

    /**
     * @brief      Tests equipment activation with failure scenarios.
     *
     *             This function tests the failure scenarios when attempting to equip
     *             items by characters with insufficient stats. It ensures that items
     *             fail to equip when the character does not meet the required conditions.
     *
     * @param[in]  index    The index of the test bot to use for equipment testing.
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_equipment_failure(uint32_t index);

    /**
     * @brief      Tests equipment overflow scenarios.
     *
     *             This function tests the overflow scenarios when attempting to equip
     *             items by characters with insufficient inventory space. It ensures that
     *             items fail to equip when the character's inventory is full.
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_equipment_overflow();
};

} // namespace fb::bot::integration

#endif // __ITEM_TEST_H__