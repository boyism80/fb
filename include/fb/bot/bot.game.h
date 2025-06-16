#ifndef __BOT_GAME_H__
#define __BOT_GAME_H__

#include <fb/bot/bot.h>
#include <fb/game/protocol.h>
#include <shared_mutex>

namespace fb::bot {

/**
 * @brief      Automated bot for testing game server functionality.
 *
 *             This bot specializes in connecting to game servers and simulating
 *             player behavior. It handles game protocols, character actions,
 *             movement patterns, combat, chat, and various game interactions.
 *             Game bots can perform automated testing of gameplay mechanics
 *             and provide load simulation for game servers.
 */
class game_bot : public base_bot
{
public:
    inline static std::shared_mutex _mutex;     ///< Mutex for thread-safe access to static members
    inline static uint32_t          _count = 0; ///< Global count of active game bot instances

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
    bool                        _inited   = false; ///< Whether the bot has been initialized
    uint32_t                    _sequence = 0;     ///< Sequence counter for protocol messages
    point<uint16_t>             _position;         ///< Current position of the bot in the game world
    fb::stream                  _transfer_buffer;  ///< Buffer for handling server transfers
    std::vector<pattern_params> _pattern_params;   ///< Collection of behavior patterns
    datetime                    _next_action_time; ///< Timestamp for the next scheduled action

public:
    /**
     * @brief      Constructs a new game bot instance.
     *
     * @param      owner  The bot container that manages this bot.
     * @param[in]  id     The unique identifier for this bot instance.
     */
    game_bot(bot_container& owner, uint32_t id);

    /**
     * @brief      Constructs a new game bot instance with parameters.
     *
     * @param      owner   The bot container that manages this bot.
     * @param[in]  id      The unique identifier for this bot instance.
     * @param[in]  params  Initialization parameters for the bot.
     */
    game_bot(bot_container& owner, uint32_t id, const fb::stream& params);

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

protected:
    /**
     * @brief      Called when the bot connects to the game server.
     *
     *             Initiates the game protocol sequence, character login,
     *             and begins automated behavior patterns.
     *
     * @return     An async task that completes when connection setup is finished.
     */
    async::task<void> on_connected() override final;

    /**
     * @brief      Called when the bot disconnects from the game server.
     *
     *             Performs cleanup and updates the global bot count.
     *
     * @return     An async task that completes when disconnection cleanup is finished.
     */
    async::task<void> on_disconnected() override final;

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
    async::task<void> on_timer(const fb::model::datetime& now) override final;

public:
    /**
     * @brief      Handles game time updates from the server.
     *
     * @param[in]  response  The time response containing current game time.
     *
     * @return     An async task that completes when time processing is finished.
     */
    async::task<void> handle_time(const fb::protocol::game::response::time& response);

    /**
     * @brief      Handles character state updates from the server.
     *
     * @param[in]  response  The internal state update response.
     *
     * @return     An async task that completes when state processing is finished.
     */
    async::task<void> handle_state(const fb::protocol::game::response::update_internal& response);

    /**
     * @brief      Handles game option updates from the server.
     *
     * @param[in]  response  The option update response.
     *
     * @return     An async task that completes when option processing is finished.
     */
    async::task<void> handle_option(const fb::protocol::game::response::option& response);

    /**
     * @brief      Handles message notifications from the server.
     *
     * @param[in]  response  The message response containing notification text.
     *
     * @return     An async task that completes when message processing is finished.
     */
    async::task<void> handle_message(const fb::protocol::game::response::message& response);

    /**
     * @brief      Handles sequence ID updates from the server.
     *
     * @param[in]  response  The ID response containing the new sequence number.
     *
     * @return     An async task that completes when sequence processing is finished.
     */
    async::task<void> handle_sequence(const fb::protocol::game::response::id& response);

    /**
     * @brief      Handles spell updates from the server.
     *
     * @param[in]  response  The spell update response.
     *
     * @return     An async task that completes when spell processing is finished.
     */
    async::task<void> handle_spell_update(const fb::protocol::game::response::spell_update& response);

    /**
     * @brief      Handles chat messages from the server.
     *
     * @param[in]  response  The chat response containing message data.
     *
     * @return     An async task that completes when chat processing is finished.
     */
    async::task<void> handle_chat(const fb::protocol::game::response::chat& response);

    /**
     * @brief      Handles action notifications from the server.
     *
     * @param[in]  response  The action response containing action data.
     *
     * @return     An async task that completes when action processing is finished.
     */
    async::task<void> handle_action(const fb::protocol::game::response::action& response);

    /**
     * @brief      Handles direction change notifications from the server.
     *
     * @param[in]  response  The direction response containing new facing direction.
     *
     * @return     An async task that completes when direction processing is finished.
     */
    async::task<void> handle_direction(const fb::protocol::game::response::direction& response);

    /**
     * @brief      Handles position updates from the server.
     *
     * @param[in]  response  The position response containing new coordinates.
     *
     * @return     An async task that completes when position processing is finished.
     */
    async::task<void> handle_position(const fb::protocol::game::response::position& response);

    /**
     * @brief      Handles movement notifications from the server.
     *
     * @param[in]  response  The move response containing movement data.
     *
     * @return     An async task that completes when movement processing is finished.
     */
    async::task<void> handle_move(const fb::protocol::game::response::move& response);

    /**
     * @brief      Handles map configuration updates from the server.
     *
     * @param[in]  response  The map config response containing map data.
     *
     * @return     An async task that completes when map processing is finished.
     */
    async::task<void> handle_map(const fb::protocol::game::response::map_config& response);

    /**
     * @brief      Handles server transfer notifications.
     *
     * @param[in]  response  The transfer response containing new server information.
     *
     * @return     An async task that completes when transfer processing is finished.
     */
    async::task<void> handle_transfer(const fb::protocol::response::transfer& response);

public:
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
     * @brief      Automated pattern for browsing board sections.
     *
     * @return     An async task that completes when board browsing is finished.
     */
    async::task<void> pattern_board_sections();
};

} // namespace fb::bot

#endif