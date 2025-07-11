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
     * @brief      Executes the item test sequence.
     *
     * @return     An async task that completes with true if all tests passed, false otherwise.
     */
    async::task<bool> execute() override;

    /**
     * @brief      Resets the test state for re-execution.
     */
    void reset() override;

    /**
     * @brief      Gets the test name.
     *
     * @return     "Item Test" as the identifier.
     */
    std::string name() const override final;

private:
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
     * @brief      Test function structure for organizing test execution.
     */
    struct test_function
    {
        std::string                                                                        name;
        std::function<async::task<bool>(std::vector<std::shared_ptr<fb::bot::game_bot>>&)> function;

        test_function(const std::string&                                                                 n,
                      std::function<async::task<bool>(std::vector<std::shared_ptr<fb::bot::game_bot>>&)> f) :
            name(n),
            function(f)
        { }
    };

private:
    std::vector<test_function> _test_functions; ///< Collection of test functions to execute

    /**
     * @brief      Initializes the test function queue.
     */
    void initialize_test_functions();

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
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_equipment_success();

    /**
     * @brief      Tests equipment activation with failure scenarios.
     *
     *             This function tests the failure scenarios when attempting to equip
     *             items by characters with insufficient stats. It ensures that items
     *             fail to equip when the character does not meet the required conditions.
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_equipment_failure();

    async::task<bool> test_equipment_overflow();

    /**
     * @brief      Resets bot state to clean initial conditions.
     *
     *             This function clears all items, resets stats to default values,
     *             removes all buffs, and restores character to a clean state for testing.
     *
     * @param      bot  The bot instance to reset.
     *
     * @return     An async task that completes when reset is finished.
     */
    async::task<void> reset_bot_state(std::shared_ptr<fb::bot::game_bot>& bot);
};

} // namespace fb::bot::integration

#endif // __ITEM_TEST_H__