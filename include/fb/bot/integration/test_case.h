#ifndef __BOT_INTEGRATION_TEST_CASE_H__
#define __BOT_INTEGRATION_TEST_CASE_H__

#include <fb/bot/game_bot.h>
#include <fb/locker.h>
#include <async/task.h>
#include <memory>
#include <vector>
#include <atomic>
#include <string>
#include <boost/asio.hpp>
#include <chrono>
#include <functional>
#include <optional>

namespace fb::bot::integration {

// Forward declarations
class game_bot_controller;

/**
 * @brief      Structure for spawned monster information
 */
struct spawned_monster_info
{
    uint32_t                   oid;      ///< Monster object ID
    fb::model::point<uint16_t> position; ///< Monster position
    uint32_t                   look;     ///< Monster look value
};

/**
 * @brief      Abstract base class for bot integration test cases.
 *
 *             Provides a framework for creating various types of integration tests
 *             such as movement, attack, skill tests, etc. Each test manages its own
 *             state, spawns bots as needed, and cleans up resources when complete.
 */
class bot_integration_test
{
public:
    virtual ~bot_integration_test() = default;

    /**
     * @brief      Initializes the test case and spawns required bots if needed.
     *
     *             Each test implementation can override this method to perform
     *             test-specific initialization and bot spawning. The default
     *             implementation does nothing, allowing tests that don't need
     *             bots to skip the spawning process.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     * @param[in]  endpoint    The server endpoint to connect bots to.
     *
     * @return     A task that completes when initialization is finished.
     */
    virtual async::task<void> initialize(game_bot_controller& controller)
    {
        // Default implementation does nothing - tests can override as needed
        co_return;
    }

    /**
     * @brief      Executes the integration test with the spawned bots.
     *
     *             This method is called after all bots are ready and starts
     *             the actual test execution logic.
     *
     * @return     A task that completes when the test finishes, returning true on success.
     */
    virtual async::task<bool> execute() = 0;

    /**
     * @brief      Checks if the test has completed.
     *
     * @return     True if the test is complete, false otherwise.
     */
    virtual bool is_complete() const = 0;

    /**
     * @brief      Resets the test state to initial conditions.
     */
    virtual void reset() = 0;

    /**
     * @brief      Gets the name of the test for logging and identification.
     *
     * @return     The test name as a string.
     */
    virtual std::string name() const = 0;

    /**
     * @brief      Checks if the test is currently in progress.
     *
     * @return     True if the test is running, false otherwise.
     */
    virtual bool is_running() const = 0;

    /**
     * @brief      Checks if all spawned bots are ready to start the test.
     *
     *             Each test implementation defines its own readiness criteria.
     *             For example, movement test might require all bots to have non-zero oid,
     *             while attack test might require bots to be in specific positions.
     *
     * @return     True if all spawned bots are ready to start this test, false otherwise.
     */
    virtual bool is_ready() const = 0;

    /**
     * @brief      Called when a bot connects to the test.
     *
     *             Each test implementation can decide whether to store this bot
     *             or ignore it based on its own requirements.
     *
     * @param[in]  bot  Shared pointer to the connected bot.
     */
    virtual void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) = 0;

    /**
     * @brief      Cleans up test resources and disconnects all spawned bots.
     *
     *             This method is called when the test completes or needs to be stopped.
     *             It ensures all bot connections are properly closed and resources are freed.
     */
    virtual void cleanup();

protected:
    /**
     * @brief      Spawns a monster at the specified position with custom validator
     *
     * @param[in]  bot       The bot that will spawn the monster
     * @param[in]  monster_name  Name of the monster to spawn (e.g., "다람쥐")
     * @param[in]  x         X coordinate for monster spawn
     * @param[in]  y         Y coordinate for monster spawn
     * @param[in]  timeout   Request timeout duration
     * @param[in]  validator Custom validator function to check spawn success
     *
     * @return     An async task that returns spawned monster information
     * @throws     std::runtime_error if monster spawning fails
     */
    async::task<spawned_monster_info>
    spawn_monster_with_validator(std::shared_ptr<fb::bot::game_bot>                               bot,
                                 const std::string&                                               monster_name,
                                 uint16_t                                                         x,
                                 uint16_t                                                         y,
                                 std::chrono::milliseconds                                        timeout,
                                 std::function<bool(const fb::protocol::game::response::update&)> validator);

    /**
     * @brief      Spawns a monster at the specified position with look validation
     *
     * @param[in]  bot       The bot that will spawn the monster
     * @param[in]  monster_name  Name of the monster to spawn (e.g., "다람쥐")
     * @param[in]  x         X coordinate for monster spawn
     * @param[in]  y         Y coordinate for monster spawn
     * @param[in]  expected_look  Expected look value for the spawned monster
     * @param[in]  timeout   Request timeout duration
     *
     * @return     An async task that returns spawned monster information
     * @throws     std::runtime_error if monster spawning fails
     */
    async::task<spawned_monster_info> spawn_monster_by_look(std::shared_ptr<fb::bot::game_bot> bot,
                                                            const std::string&                 monster_name,
                                                            uint16_t                           x,
                                                            uint16_t                           y,
                                                            uint32_t                           expected_look,
                                                            std::chrono::milliseconds          timeout);

    /**
     * @brief      Spawns multiple monsters at relative positions from a bot with custom validator
     *
     * @param[in]  bot       The bot that will spawn the monsters
     * @param[in]  monster_name  Name of the monster to spawn
     * @param[in]  relative_positions  Vector of {x, y} relative positions from bot
     * @param[in]  timeout   Request timeout duration
     * @param[in]  validator Custom validator function to check spawn success
     *
     * @return     An async task that returns vector of spawned monster information
     * @throws     std::runtime_error if any monster spawning fails
     */
    async::task<std::vector<spawned_monster_info>>
    spawn_monsters_relative_with_validator(std::shared_ptr<fb::bot::game_bot>      bot,
                                           const std::string&                      monster_name,
                                           const std::vector<std::pair<int, int>>& relative_positions,
                                           std::chrono::milliseconds               timeout,
                                           std::function<bool(const fb::protocol::game::response::update&)> validator);

    /**
     * @brief      Spawns multiple monsters at relative positions from a bot with look validation
     *
     * @param[in]  bot       The bot that will spawn the monsters
     * @param[in]  monster_name  Name of the monster to spawn
     * @param[in]  relative_positions  Vector of {x, y} relative positions from bot
     * @param[in]  expected_look  Expected look value for the spawned monsters
     * @param[in]  timeout   Request timeout duration
     *
     * @return     An async task that returns vector of spawned monster information
     * @throws     std::runtime_error if any monster spawning fails
     */
    async::task<std::vector<spawned_monster_info>>
    spawn_monsters_relative_by_look(std::shared_ptr<fb::bot::game_bot>      bot,
                                    const std::string&                      monster_name,
                                    const std::vector<std::pair<int, int>>& relative_positions,
                                    uint32_t                                expected_look,
                                    std::chrono::milliseconds               timeout);

    /**
     * @brief      Spawns a single monster at relative position from a bot with look validation
     *
     * @param[in]  bot       The bot that will spawn the monster
     * @param[in]  monster_name  Name of the monster to spawn
     * @param[in]  relative_x  X coordinate relative to bot position
     * @param[in]  relative_y  Y coordinate relative to bot position
     * @param[in]  expected_look  Expected look value for the spawned monster
     * @param[in]  timeout   Request timeout duration
     *
     * @return     An async task that returns spawned monster information
     * @throws     std::runtime_error if monster spawning fails
     */
    async::task<spawned_monster_info> spawn_monster_relative_by_look(std::shared_ptr<fb::bot::game_bot> bot,
                                                                     const std::string&                 monster_name,
                                                                     int                                relative_x,
                                                                     int                                relative_y,
                                                                     uint32_t                           expected_look,
                                                                     std::chrono::milliseconds          timeout);

    /**
     * @brief      Sets maximum HP and MP for a bot
     *
     * @param[in]  bot       The bot to modify
     * @param[in]  max_hp    Maximum HP value to set
     * @param[in]  max_mp    Maximum MP value to set
     * @param[in]  timeout   Request timeout duration
     *
     * @return     An async task that returns true if HP/MP were set successfully
     */
    async::task<bool>
    set_max_hp_mp(std::shared_ptr<fb::bot::game_bot> bot, int max_hp, int max_mp, std::chrono::milliseconds timeout);

    /**
     * @brief      Sets current HP and MP for a bot
     *
     * @param[in]  bot       The bot to modify
     * @param[in]  current_hp  Current HP value to set
     * @param[in]  current_mp  Current MP value to set
     * @param[in]  timeout   Request timeout duration
     *
     * @return     An async task that returns true if HP/MP were set successfully
     */
    async::task<bool> set_current_hp_mp(std::shared_ptr<fb::bot::game_bot> bot,
                                        int                                current_hp,
                                        int                                current_mp,
                                        std::chrono::milliseconds          timeout);

    /**
     * @brief      Sets HP and MP for multiple bots
     *
     * @param[in]  bots      Vector of bots to modify
     * @param[in]  max_hp    Maximum HP value to set
     * @param[in]  max_mp    Maximum MP value to set
     * @param[in]  current_hp  Current HP value to set (optional, uses max_hp if not specified)
     * @param[in]  current_mp  Current MP value to set (optional, uses max_mp if not specified)
     * @param[in]  timeout   Request timeout duration
     *
     * @return     An async task that returns true if all bots were set successfully
     */
    async::task<bool> setup_bots_hp_mp(const std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                       int                                                    max_hp,
                                       int                                                    max_mp,
                                       std::optional<int>                                     current_hp = std::nullopt,
                                       std::optional<int>                                     current_mp = std::nullopt,
                                       std::chrono::milliseconds                              timeout    = 5s);

    /**
     * @brief      Learns multiple spells for a bot
     *
     * @param[in]  bot       The bot that will learn the spells
     * @param[in]  spell_names  Vector of spell names to learn
     * @param[in]  timeout   Request timeout duration
     *
     * @return     An async task that returns the number of successfully learned spells
     */
    async::task<size_t> learn_spells(std::shared_ptr<fb::bot::game_bot> bot,
                                     const std::vector<std::string>&    spell_names,
                                     std::chrono::milliseconds          timeout);

    /**
     * @brief      Removes all spells from a bot
     *
     * @param[in]  bot       The bot that will have spells removed
     * @param[in]  timeout   Request timeout duration
     *
     * @return     An async task that returns true if spells were removed successfully
     */
    async::task<bool> clear_all_spells(std::shared_ptr<fb::bot::game_bot> bot, std::chrono::milliseconds timeout);

    /**
     * @brief      Moves a bot back to its original position after movement spells
     *
     * @param[in]  bot       The bot to move back
     * @param[in]  original_position  The original position to return to
     * @param[in]  interval  Delay between movement commands
     *
     * @return     An async task that completes when the bot has returned to original position
     */
    async::task<void> move_bot_back_to_position(std::shared_ptr<fb::bot::game_bot> bot,
                                                const fb::model::point<uint16_t>&  original_position,
                                                std::chrono::milliseconds          interval = 100ms);
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_TEST_CASE_H__