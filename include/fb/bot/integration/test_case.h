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
 *             Tests can be chained together to form a linked list for sequential execution.
 */
class bot_integration_test
{
public:
    enum class test_state : uint8_t
    {
        idle,      ///< Test is not started yet
        ready,     ///< Test is ready to start
        running,   ///< Test is currently executing
        completed, ///< Test has completed successfully
        failed     ///< Test has failed
    };

protected:
    game_bot_controller&                            _controller; ///< Reference to the parent game bot controller
    std::vector<std::shared_ptr<fb::bot::game_bot>> _test_bots;  ///< Collection of bots managed by this test
    test_state                                      _state{test_state::idle}; ///< Current state of the test

protected:
    // No additional protected members

public:
    /**
     * @brief      Constructs a new bot integration test with controller reference.
     *
     * @param[in]  controller  Reference to the game bot controller.
     */
    bot_integration_test(game_bot_controller& controller) :
        _controller(controller)
    { }

    virtual ~bot_integration_test() = default;

    /**
     * @brief      Gets the current state of the test.
     *
     * @return     The current test state.
     */
    test_state get_state() const
    {
        return this->_state;
    }

    /**
     * @brief      Sets the current state of the test.
     *
     * @param[in]  state  The new test state.
     */
    void set_state(test_state state)
    {
        this->_state = state;
    }

    /**
     * @brief      Checks if the test has completed execution.
     *
     * @return     True if the test is complete, false otherwise.
     */
    bool is_complete() const
    {
        return this->_state == test_state::completed;
    }

    /**
     * @brief      Checks if the test is currently running.
     *
     * @return     True if the test is running, false otherwise.
     */
    bool is_running() const
    {
        return this->_state == test_state::running;
    }

    /**
     * @brief      Called when a bot connects to the test.
     *
     *             Default implementation adds the bot to the test's bot collection.
     *             Can be overridden by derived classes for custom behavior.
     *
     * @param[in]  bot  Shared pointer to the connected bot.
     */
    virtual void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
    {
        this->_test_bots.push_back(bot);
    }

    /**
     * @brief      Called when a bot receives an object ID response.
     *
     *             This method is called by the hook system when a bot receives
     *             an object ID. Derived classes should override this to check
     *             readiness and notify the controller.
     *
     * @param[in]  bot       The bot that received the object ID.
     * @param[in]  response  The object ID response.
     *
     * @return     An async task that completes when processing is finished.
     */
    virtual async::task<void> on_hook_sequence(fb::bot::game_bot& bot, const fb::protocol::game::response::id& response)
    {
        // Default implementation does nothing
        co_return;
    }

    /**
     * @brief      Notifies the controller that this test is ready to start.
     *
     *             Called by derived classes when they detect they are ready to begin execution.
     */
    void notify_ready();

    /**
     * @brief      Performs cleanup operations when the test is destroyed.
     *
     *             Default implementation does nothing. Can be overridden
     *             by derived classes for custom cleanup logic.
     */
    virtual void cleanup()
    {
        // Default implementation does nothing
    }

    /**
     * @brief      Checks if the test is ready to start execution.
     *
     *             This method should be overridden by derived classes to
     *             implement specific readiness checks.
     *
     * @return     True if the test is ready to start, false otherwise.
     */
    virtual bool is_ready() const
    {
        // Default implementation: ready if we have at least one bot
        return !this->_test_bots.empty();
    }

    /**
     * @brief      Gets the current list of test bots.
     *
     * @return     Vector of shared pointers to the test bots.
     */
    std::vector<std::shared_ptr<fb::bot::game_bot>> get_test_bots() const
    {
        return this->_test_bots;
    }

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
     * @brief      Gets the name of the test for logging and identification.
     *
     * @return     The test name as a string.
     */
    virtual std::string name() const = 0;

protected:
    /**
     * @brief      Resets the test state to idle.
     *
     *             This method should be overridden by derived classes to
     *             implement specific reset logic.
     */
    virtual void reset()
    {
        this->_state = test_state::idle;
    }

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

    async::task<bool> setup_bot_stats(std::shared_ptr<fb::bot::game_bot> bot,
                                      int                                max_hp,
                                      int                                max_mp,
                                      std::optional<int>                 current_hp,
                                      std::optional<int>                 current_mp,
                                      std::chrono::milliseconds          timeout);

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
                                                const fb::model::timespan&         interval = 100ms);
};

} // namespace fb::bot::integration

#endif // !__BOT_INTEGRATION_TEST_CASE_H__