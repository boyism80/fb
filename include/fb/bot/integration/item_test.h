#ifndef __ITEM_TEST_H__
#define __ITEM_TEST_H__

#include <fb/bot/integration/bot_integration_test.h>
#include <fb/bot/game_bot.h>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

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
     * @brief      Initializes the item test with required bots.
     *
     * @param      controller  Reference to the game bot controller.
     *
     * @return     An async task that completes when initialization is finished.
     */
    static async::task<void> initialize(game_bot_controller& controller);

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
     * @brief      Checks if the test is ready to execute.
     *
     * @return     True if ready, false otherwise.
     */
    bool is_ready() const override;

    /**
     * @brief      Handles bot connection events.
     *
     * @param[in]  bot  The connected bot instance.
     */
    void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) override;

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

    /**
     * @brief      Hook handler for object ID responses.
     *
     * @param      bot       The bot that received the response.
     * @param[in]  response  The ID response data.
     *
     * @return     An async task that completes when handling is finished.
     */
    async::task<void> on_hook_sequence(fb::bot::game_bot& bot, const fb::protocol::game::response::id& response);

    /**
     * @brief      Hook handler for position responses.
     *
     * @param      bot       The bot that received the response.
     * @param[in]  response  The position response data.
     *
     * @return     An async task that completes when handling is finished.
     */
    async::task<void> on_hook_position(fb::bot::game_bot& bot, const fb::protocol::game::response::position& response);

    // Test function implementations
    /**
     * @brief      Tests equipment activation with success scenarios.
     * Tests that items can be properly equipped by characters with appropriate stats
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_equipment_success();

    /**
     * @brief      Tests equipment activation with failure scenarios.
     * Tests that items fail to equip when characters have insufficient stats
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_equipment_failure();

    /**
     * @brief      Tests equipment deactivation (unequip) functionality.
     * Tests that items can be properly unequipped/deactivated
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_equipment_deactivation();

    /**
     * @brief      Tests gender-specific item equipping.
     * Tests that gender-specific items can only be equipped by appropriate characters
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_gender_specific_items();

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