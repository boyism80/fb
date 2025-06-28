#ifndef __BOT_GAME_H__
#define __BOT_GAME_H__

#include <fb/bot/bot.h>
#include <fb/game/protocol.h>
#include <shared_mutex>

namespace fb::bot {

// Forward declarations
class game_bot_controller;
template <typename ControllerType> class bot;

/**
 * @brief      Automated bot for testing game server functionality.
 *
 *             This bot specializes in connecting to game servers and simulating
 *             player behavior. It handles game protocols, character actions,
 *             movement patterns, combat, chat, and various game interactions.
 *             Game bots can perform automated testing of gameplay mechanics
 *             and provide load simulation for game servers.
 */
class game_bot : public bot<game_bot>
{
public:
    using bot_controller_type = game_bot_controller; ///< Type alias for the bot_controller type

private:
    /**
     * @brief      Parameters for defining bot behavior patterns.
     *
     *             This structure encapsulates the configuration for automated
     *             bot actions, including the action function and timing constraints.
     */
    typedef struct _pattern_params_tag
    {
        std::function<async::task<void>()>  fn;  ///< The action function to execute
        std::chrono::steady_clock::duration min; ///< Minimum delay before action
        std::chrono::steady_clock::duration max; ///< Maximum delay before action
    } pattern_params;

private:
    bool                        _inited = false;   ///< Whether the bot has been initialized
    std::vector<pattern_params> _pattern_params;   ///< Collection of behavior patterns
    datetime                    _next_action_time; ///< Timestamp for the next scheduled action

public:
    /**
     * @brief      Constructs a new game bot instance.
     *
     * @param      bot_controller  The game bot bot_controller that manages this bot.
     * @param[in]  id          The unique identifier for this bot instance.
     */
    game_bot(bot_controller<game_bot>& bot_controller, uint32_t id);

    /**
     * @brief      Constructs a new game bot instance with transfer parameters.
     *
     * @param      bot_controller  The game bot bot_controller that manages this bot.
     * @param[in]  id          The unique identifier for this bot instance.
     * @param[in]  params      Transfer parameters containing crypto information.
     */
    game_bot(bot_controller<game_bot>& bot_controller, uint32_t id, const fb::stream& params);

    /**
     * @brief      Destroys the game bot and performs cleanup.
     */
    ~game_bot();

private:
    /**
     * @brief      Registers a behavior pattern for the bot.
     *
     *             Adds an automated action to the bot's behavior repertoire
     *             with specified timing constraints for execution.
     *
     * @param[in]  fn   The member function to execute as a pattern.
     * @param[in]  min  The minimum delay before executing the action.
     * @param[in]  max  The maximum delay before executing the action.
     *
     * @tparam     Class  The class type containing the member function.
     */
    template <typename Class>
    void pattern(async::task<void> (Class::*fn)(),
                 const std::chrono::steady_clock::duration& min,
                 const std::chrono::steady_clock::duration& max)
    {
        this->_pattern_params.push_back(pattern_params{std::bind(fn, static_cast<Class*>(this)), min, max});
    }

public:
    /**
     * @brief      Handles timer events for automated bot behavior.
     *
     *             Executes scheduled behavior patterns based on timing
     *             constraints and current game state.
     *
     * @param[in]  now  The current date and time.
     *
     * @return     An async task that completes when timer processing is finished.
     */
    async::task<void> process_random_pattern(const fb::model::datetime& now);

public:
    // Game state accessors
    uint32_t sequence() const
    {
        return _sequence;
    }
    void set_sequence(uint32_t value)
    {
        _sequence = value;
    }

    point<uint16_t> position() const
    {
        return _position;
    }
    void set_position(const point<uint16_t>& value)
    {
        _position = value;
    }

    bool is_initialized() const
    {
        return _inited;
    }
    void set_initialized(bool value)
    {
        _inited = value;
    }

    const fb::stream& transfer_buffer() const
    {
        return _transfer_buffer;
    }

    /**
     * @brief      Automated pattern for sending chat messages.
     *
     * @return     An async task that completes when chat action is finished.
     */
    async::task<void> pattern_chat();

    /**
     * @brief      Automated pattern for performing attack actions.
     *
     * @return     An async task that completes when attack action is finished.
     */
    async::task<void> pattern_attack();

    /**
     * @brief      Automated pattern for changing facing direction.
     *
     * @return     An async task that completes when direction action is finished.
     */
    async::task<void> pattern_direction();

    /**
     * @brief      Automated pattern for character movement.
     *
     * @return     An async task that completes when movement action is finished.
     */
    async::task<void> pattern_move();

    /**
     * @brief      Automated pattern for picking up items.
     *
     * @return     An async task that completes when pickup action is finished.
     */
    async::task<void> pattern_pickup();

    /**
     * @brief      Automated pattern for displaying emotions.
     *
     * @return     An async task that completes when emotion action is finished.
     */
    async::task<void> pattern_emotion();

    /**
     * @brief      Automated pattern for browsing bulletin sections.
     *
     * @return     An async task that completes when bulletin browsing is finished.
     */
    async::task<void> pattern_bulletin_sections();

private:
    uint32_t        _sequence = 0;    ///< Sequence counter for protocol messages
    point<uint16_t> _position;        ///< Current position of the bot in the game world
    fb::stream      _transfer_buffer; ///< Buffer for handling server transfers
};

} // namespace fb::bot

#endif