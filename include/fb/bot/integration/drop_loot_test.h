#ifndef __BOT_INTEGRATION_DROP_LOOT_TEST_H__
#define __BOT_INTEGRATION_DROP_LOOT_TEST_H__

#include <fb/bot/integration/test_case.h>

namespace fb::bot::integration {

/**
 * @brief      Drop and loot test class for testing item dropping and looting functionality.
 *
 *             This class tests various drop and loot scenarios including:
 *             - Basic item drop and loot operations
 *             - Inventory capacity limitations
 *             - Item stack overflow handling
 *             - Money overflow handling
 *             - PK-related loot mechanics
 *
 * @author     Bot Integration Team
 * @date       2024
 */
class drop_loot_test : public bot_integration_test
{
public:
    /**
     * @brief      Constructs a drop_loot_test instance.
     *
     * @param[in]  controller  The game bot controller for managing bots.
     */
    drop_loot_test(game_bot_controller& controller);

    /**
     * @brief      Default destructor.
     */
    ~drop_loot_test() = default;

public:
    /**
     * @brief      Executes all drop and loot test scenarios.
     *
     * @return     An async task that returns true if all tests pass, false otherwise.
     */
    async::task<bool> execute() override;

    /**
     * @brief      Resets the test state.
     */
    void reset() override;

    /**
     * @brief      Gets the name of this test.
     *
     * @return     The test name as a string.
     */
    std::string name() const override;

private:
    // Test scenarios
    /**
     * @brief      Tests basic item drop and loot functionality.
     *
     * @param[in]  bot1  First bot for testing.
     * @param[in]  bot2  Second bot for testing.
     * @return     An async task that returns true if test passes.
     */
    async::task<bool> test_scenario_1(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);

    /**
     * @brief      Tests inventory full item loot failure.
     *
     * @param[in]  bot1  First bot for testing.
     * @param[in]  bot2  Second bot for testing.
     * @return     An async task that returns true if test passes.
     */
    async::task<bool> test_scenario_2(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);

    /**
     * @brief      Tests item stack overflow loot failure.
     *
     * @param[in]  bot1  First bot for testing.
     * @param[in]  bot2  Second bot for testing.
     * @return     An async task that returns true if test passes.
     */
    async::task<bool> test_scenario_3(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);

    /**
     * @brief      Tests money overflow loot failure.
     *
     * @param[in]  bot1  First bot for testing.
     * @param[in]  bot2  Second bot for testing.
     * @return     An async task that returns true if test passes.
     */
    async::task<bool> test_scenario_4(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);

    /**
     * @brief      Tests PK-related loot mechanics with hellfire spell.
     *
     * @param[in]  bot1  First bot for testing.
     * @param[in]  bot2  Second bot for testing.
     * @return     An async task that returns true if test passes.
     */
    async::task<bool> test_scenario_5(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2);

private:
    // Helper functions
    /**
     * @brief      Resets bot state for clean testing.
     *
     * @param[in]  bot  The bot to reset.
     * @return     An async task.
     */
    async::task<void> reset_bot_state(std::shared_ptr<game_bot>& bot);

    /**
     * @brief      Checks if bot has a specific item.
     *
     * @param[in]  bot   The bot to check.
     * @param[in]  name  The item name to search for.
     * @return     True if bot has the item, false otherwise.
     */
    bool has_item(const std::shared_ptr<game_bot>& bot, const std::string& name);

    /**
     * @brief      Gets the count of a specific item in bot's inventory.
     *
     * @param[in]  bot   The bot to check.
     * @param[in]  name  The item name to search for.
     * @return     The item count.
     */
    uint16_t get_item_count(const std::shared_ptr<game_bot>& bot, const std::string& name);

    /**
     * @brief      Gets the inventory slot index of a specific item.
     *
     * @param[in]  bot   The bot to check.
     * @param[in]  name  The item name to search for.
     * @return     The slot index, or 0xFF if not found.
     */
    uint8_t get_item_slot(const std::shared_ptr<game_bot>& bot, const std::string& name);
};

} // namespace fb::bot::integration

#endif //__BOT_INTEGRATION_DROP_LOOT_TEST_H__