#ifndef __BOT_GAME_H__
#define __BOT_GAME_H__

#include <fb/bot/bot.h>
#include <fb/game/protocol.h>
#include <shared_mutex>
#include <set>
#include <string>
#include <map>
#include <optional>

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

    /**
     * @brief      Simple item structure for bot inventory tracking.
     */
    struct simple_item
    {
        std::string name;      ///< Item name
        uint32_t    count = 0; ///< Item count

        simple_item() = default;
        simple_item(const std::string& n, uint32_t c) :
            name(n),
            count(c)
        { }
    };

    /**
     * @brief      Simple spell structure for bot spell tracking.
     */
    struct simple_spell
    {
        std::string name; ///< Spell name
        uint8_t     type; ///< Spell type

        simple_spell() = default;
        simple_spell(const std::string& n, uint8_t t) :
            name(n),
            type(t)
        { }
    };

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
    // Bot control and pattern management
    bool                        _inited = false;   ///< Whether the bot has been initialized
    std::vector<pattern_params> _pattern_params;   ///< Collection of behavior patterns
    datetime                    _next_action_time; ///< Timestamp for the next scheduled action

    // Basic bot state
    uint32_t        _oid = 0;                       ///< Object ID for protocol messages
    point<uint16_t> _position;                      ///< Current position of the bot in the game world
    fb::stream      _transfer_buffer;               ///< Buffer for handling server transfers
    DIRECTION       _direction = DIRECTION::BOTTOM; ///< Current facing direction of the bot

    // Additional bot state information
    uint16_t              _look  = 0;     ///< Visual appearance ID of the bot
    uint8_t               _color = 0;     ///< Color value of the bot
    bool                  _dead  = false; ///< Whether the bot is dead
    std::set<std::string> _active_buffs;  ///< Set of currently active buff names

    // Simple item and spell management
    std::map<uint8_t, simple_item>  _items;  ///< Map of slot index to item info
    std::map<uint8_t, simple_spell> _spells; ///< Map of slot index to spell info

    // Character internal state information from update_internal
    uint8_t  _nation        = 0; ///< Character's nation
    uint8_t  _creature      = 0; ///< Character's creature type
    uint8_t  _level         = 0; ///< Character's level
    uint32_t _base_hp       = 0; ///< Character's base HP
    uint32_t _base_mp       = 0; ///< Character's base MP
    uint8_t  _strength      = 0; ///< Character's strength stat
    uint8_t  _intelligence  = 0; ///< Character's intelligence stat
    uint8_t  _dexterity     = 0; ///< Character's dexterity stat
    uint32_t _hp            = 0; ///< Character's current HP
    uint32_t _mp            = 0; ///< Character's current MP
    uint32_t _exp           = 0; ///< Character's experience points
    uint32_t _money         = 0; ///< Character's money
    uint32_t _crowd_control = 0; ///< Character's crowd control status
    uint8_t  _mail_count    = 0; ///< Number of unread mails
    uint8_t  _fast_move     = 0; ///< Fast move option setting

    // Character external appearance information from update_external
    uint8_t     _disguised    = 0; ///< Character's disguise state
    uint8_t     _sex          = 0; ///< Character's sex
    uint8_t     _state        = 0; ///< Character's current state (dead, paralyzed, etc)
    uint8_t     _armor_dress  = 0; ///< Armor appearance ID
    uint8_t     _armor_color  = 0; ///< Armor color
    uint16_t    _weapon_dress = 0; ///< Weapon appearance ID
    uint8_t     _weapon_color = 0; ///< Weapon color
    uint8_t     _shield_dress = 0; ///< Shield appearance ID
    uint8_t     _shield_color = 0; ///< Shield color
    uint8_t     _head_marker  = 0; ///< Head marker (clan mark, etc)
    std::string _name;             ///< Character name

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
    uint32_t oid() const;
    void     set_oid(uint32_t value);

    point<uint16_t> position() const;
    void            set_position(const point<uint16_t>& value);

    bool is_initialized() const;
    void set_initialized(bool value);

    const fb::stream& transfer_buffer() const;

    DIRECTION direction() const;
    void      set_direction(DIRECTION value);

    // Additional state accessors
    uint16_t look() const;
    void     set_look(uint16_t value);

    uint8_t color() const;
    void    set_color(uint8_t value);

    bool is_dead() const;
    void set_dead(bool value);

    // Buff management methods
    const std::set<std::string>& active_buffs() const;
    void                         add_buff(const std::string& name);
    void                         remove_buff(const std::string& name);
    bool                         has_buff(const std::string& name) const;

    // Spell management methods
    void                                   update_spell(uint8_t slot, const std::string& name, uint8_t type);
    void                                   remove_spell(uint8_t slot);
    std::optional<simple_spell>            get_spell(uint8_t slot) const;
    bool                                   has_spell(uint8_t slot) const;
    const std::map<uint8_t, simple_spell>& spells() const;

    // Character state accessors
    /**
     * @brief      Gets the character's nation.
     *
     * @return     The nation ID.
     */
    uint8_t nation() const;

    /**
     * @brief      Sets the character's nation.
     *
     * @param[in]  value  The nation ID to set.
     */
    void set_nation(uint8_t value);

    /**
     * @brief      Gets the character's creature type.
     *
     * @return     The creature type ID.
     */
    uint8_t creature() const;

    /**
     * @brief      Sets the character's creature type.
     *
     * @param[in]  value  The creature type ID to set.
     */
    void set_creature(uint8_t value);

    /**
     * @brief      Gets the character's level.
     *
     * @return     The character level.
     */
    uint8_t level() const;

    /**
     * @brief      Sets the character's level.
     *
     * @param[in]  value  The character level to set.
     */
    void set_level(uint8_t value);

    /**
     * @brief      Gets the character's base HP.
     *
     * @return     The base HP value.
     */
    uint32_t base_hp() const;

    /**
     * @brief      Sets the character's base HP.
     *
     * @param[in]  value  The base HP value to set.
     */
    void set_base_hp(uint32_t value);

    /**
     * @brief      Gets the character's base MP.
     *
     * @return     The base MP value.
     */
    uint32_t base_mp() const;

    /**
     * @brief      Sets the character's base MP.
     *
     * @param[in]  value  The base MP value to set.
     */
    void set_base_mp(uint32_t value);

    /**
     * @brief      Gets the character's strength stat.
     *
     * @return     The strength value.
     */
    uint8_t strength() const;

    /**
     * @brief      Sets the character's strength stat.
     *
     * @param[in]  value  The strength value to set.
     */
    void set_strength(uint8_t value);

    /**
     * @brief      Gets the character's intelligence stat.
     *
     * @return     The intelligence value.
     */
    uint8_t intelligence() const;

    /**
     * @brief      Sets the character's intelligence stat.
     *
     * @param[in]  value  The intelligence value to set.
     */
    void set_intelligence(uint8_t value);

    /**
     * @brief      Gets the character's dexterity stat.
     *
     * @return     The dexterity value.
     */
    uint8_t dexterity() const;

    /**
     * @brief      Sets the character's dexterity stat.
     *
     * @param[in]  value  The dexterity value to set.
     */
    void set_dexterity(uint8_t value);

    /**
     * @brief      Gets the character's current HP.
     *
     * @return     The current HP value.
     */
    uint32_t hp() const;

    /**
     * @brief      Sets the character's current HP.
     *
     * @param[in]  value  The current HP value to set.
     */
    void set_hp(uint32_t value);

    /**
     * @brief      Gets the character's current MP.
     *
     * @return     The current MP value.
     */
    uint32_t mp() const;

    /**
     * @brief      Sets the character's current MP.
     *
     * @param[in]  value  The current MP value to set.
     */
    void set_mp(uint32_t value);

    /**
     * @brief      Gets the character's experience points.
     *
     * @return     The experience points value.
     */
    uint32_t exp() const;

    /**
     * @brief      Sets the character's experience points.
     *
     * @param[in]  value  The experience points value to set.
     */
    void set_exp(uint32_t value);

    /**
     * @brief      Gets the character's money.
     *
     * @return     The money amount.
     */
    uint32_t money() const;

    /**
     * @brief      Sets the character's money.
     *
     * @param[in]  value  The money amount to set.
     */
    void set_money(uint32_t value);

    /**
     * @brief      Gets the character's crowd control status.
     *
     * @return     The crowd control status value.
     */
    uint32_t crowd_control() const;

    /**
     * @brief      Sets the character's crowd control status.
     *
     * @param[in]  value  The crowd control status value to set.
     */
    void set_crowd_control(uint32_t value);

    /**
     * @brief      Gets the character's unread mail count.
     *
     * @return     The number of unread mails.
     */
    uint8_t mail_count() const;

    /**
     * @brief      Sets the character's unread mail count.
     *
     * @param[in]  value  The number of unread mails to set.
     */
    void set_mail_count(uint8_t value);

    /**
     * @brief      Gets the character's fast move setting.
     *
     * @return     The fast move option value.
     */
    uint8_t fast_move() const;

    /**
     * @brief      Sets the character's fast move setting.
     *
     * @param[in]  value  The fast move option value to set.
     */
    void set_fast_move(uint8_t value);

    /**
     * @brief      Gets the character's disguise state.
     *
     * @return     The disguise state value (0 = not disguised, 1 = disguised).
     */
    uint8_t disguised() const;

    /**
     * @brief      Sets the character's disguise state.
     *
     * @param[in]  value  The disguise state value to set.
     */
    void set_disguised(uint8_t value);

    /**
     * @brief      Gets the character's sex.
     *
     * @return     The sex value (0 = male, 1 = female).
     */
    uint8_t sex() const;

    /**
     * @brief      Sets the character's sex.
     *
     * @param[in]  value  The sex value to set.
     */
    void set_sex(uint8_t value);

    /**
     * @brief      Gets the character's current state.
     *
     * @return     The state value (dead, paralyzed, etc.).
     */
    uint8_t state() const;

    /**
     * @brief      Sets the character's current state.
     *
     * @param[in]  value  The state value to set.
     */
    void set_state(uint8_t value);

    /**
     * @brief      Gets the character's armor appearance ID.
     *
     * @return     The armor appearance ID.
     */
    uint8_t armor_dress() const;

    /**
     * @brief      Sets the character's armor appearance ID.
     *
     * @param[in]  value  The armor appearance ID to set.
     */
    void set_armor_dress(uint8_t value);

    /**
     * @brief      Gets the character's armor color.
     *
     * @return     The armor color value.
     */
    uint8_t armor_color() const;

    /**
     * @brief      Sets the character's armor color.
     *
     * @param[in]  value  The armor color value to set.
     */
    void set_armor_color(uint8_t value);

    /**
     * @brief      Gets the character's weapon appearance ID.
     *
     * @return     The weapon appearance ID.
     */
    uint16_t weapon_dress() const;

    /**
     * @brief      Sets the character's weapon appearance ID.
     *
     * @param[in]  value  The weapon appearance ID to set.
     */
    void set_weapon_dress(uint16_t value);

    /**
     * @brief      Gets the character's weapon color.
     *
     * @return     The weapon color value.
     */
    uint8_t weapon_color() const;

    /**
     * @brief      Sets the character's weapon color.
     *
     * @param[in]  value  The weapon color value to set.
     */
    void set_weapon_color(uint8_t value);

    /**
     * @brief      Gets the character's shield appearance ID.
     *
     * @return     The shield appearance ID.
     */
    uint8_t shield_dress() const;

    /**
     * @brief      Sets the character's shield appearance ID.
     *
     * @param[in]  value  The shield appearance ID to set.
     */
    void set_shield_dress(uint8_t value);

    /**
     * @brief      Gets the character's shield color.
     *
     * @return     The shield color value.
     */
    uint8_t shield_color() const;

    /**
     * @brief      Sets the character's shield color.
     *
     * @param[in]  value  The shield color value to set.
     */
    void set_shield_color(uint8_t value);

    /**
     * @brief      Gets the character's head marker.
     *
     * @return     The head marker value (clan mark, etc.).
     */
    uint8_t head_marker() const;

    /**
     * @brief      Sets the character's head marker.
     *
     * @param[in]  value  The head marker value to set.
     */
    void set_head_marker(uint8_t value);

    /**
     * @brief      Gets the character's name.
     *
     * @return     A constant reference to the character name string.
     */
    const std::string& name() const;

    /**
     * @brief      Sets the character's name.
     *
     * @param[in]  value  The character name to set.
     */
    void set_name(const std::string& value);

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

public:
    // Simple item management methods

    /**
     * @brief      Updates or adds an item to the inventory.
     *
     * @param[in]  slot   The inventory slot index.
     * @param[in]  name   The item name.
     * @param[in]  count  The item count.
     */
    void update_item(uint8_t slot, const std::string& name, uint32_t count);

    /**
     * @brief      Removes an item from the inventory.
     *
     * @param[in]  slot  The inventory slot index to remove.
     */
    void remove_item(uint8_t slot);

    /**
     * @brief      Gets an item from the inventory by slot.
     *
     * @param[in]  slot  The inventory slot index.
     *
     * @return     An optional containing the item if found, otherwise nullopt.
     */
    std::optional<simple_item> get_item(uint8_t slot) const;

    /**
     * @brief      Checks if an inventory slot has an item.
     *
     * @param[in]  slot  The inventory slot index to check.
     *
     * @return     True if the slot has an item, false otherwise.
     */
    bool has_item(uint8_t slot) const;

    /**
     * @brief      Gets all items in the inventory.
     *
     * @return     A constant reference to the items map.
     */
    const std::map<uint8_t, simple_item>& items() const;
};

} // namespace fb::bot

#endif