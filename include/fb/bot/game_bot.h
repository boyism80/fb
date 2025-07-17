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

struct spawned_monster_info
{
    uint32_t                   oid;      ///< Monster object ID
    fb::model::point<uint16_t> position; ///< Monster position
    uint32_t                   look;     ///< Monster look value
};

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

        /**
         * @brief      Check if this item is equipment type.
         *
         * @param[in]  controller  Reference to the game bot controller for model access.
         *
         * @return     True if the item is equipment, false otherwise.
         */
        bool is_equipment(const game_bot_controller& controller) const;

        /**
         * @brief      Get equipment information as formatted string.
         *
         *             This function generates a formatted string containing equipment
         *             information similar to the tip_message() function in equipment.cpp.
         *
         * @param[in]  controller  Reference to the game bot controller for model access.
         *
         * @return     Formatted string with equipment information, or empty string if not equipment.
         */
        std::string get_equipment_info(const game_bot_controller& controller) const;
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
    uint16_t        _map = 0xFFFF;                  ///< Current map ID
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
    uint8_t     _disguised    = 0;             ///< Character's disguise state
    SEX         _sex          = SEX::MAN;      ///< Character's sex
    STATE       _state        = STATE::NORMAL; ///< Character's current state (dead, paralyzed, etc)
    uint8_t     _armor_dress  = 0;             ///< Armor appearance ID
    uint8_t     _armor_color  = 0;             ///< Armor color
    uint16_t    _weapon_dress = 0;             ///< Weapon appearance ID
    uint8_t     _weapon_color = 0;             ///< Weapon color
    uint8_t     _shield_dress = 0;             ///< Shield appearance ID
    uint8_t     _shield_color = 0;             ///< Shield color
    uint8_t     _head_marker  = 0;             ///< Head marker (clan mark, etc)
    std::string _name;                         ///< Character name

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
     * @param[in]  params      Transfer parameters containing encryption information.
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

    uint16_t map() const;
    void     set_map(uint16_t value);

    point<uint16_t> position() const;
    void            set_position(const point<uint16_t>& value);

    bool inited() const;
    void inited(bool value);

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
    SEX sex() const;

    /**
     * @brief      Sets the character's sex.
     *
     * @param[in]  value  The sex value to set.
     */
    void set_sex(SEX value);

    /**
     * @brief      Gets the character's current state.
     *
     * @return     The state value (dead, paralyzed, etc.).
     */
    STATE state() const;

    /**
     * @brief      Sets the character's current state.
     *
     * @param[in]  value  The state value to set.
     */
    void set_state(STATE value);

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
     * @brief      Move the bot in the specified direction.
     *
     * @param[in]  direction  The direction to move.
     * @param[in]  step       The number of steps to move.
     * @param[in]  delay      The delay between steps.
     */
    async::task<void> move(DIRECTION direction, int step = 1, const fb::model::timespan& delay = 0ms);

    /**
     * @brief      Move the bot to a specific map and coordinates.
     *
     *             Executes the '/맵이동 map x y' command and waits for the map_bgm
     *             response to confirm successful map transition.
     *
     * @param[in]  map_name  The name of the target map.
     * @param[in]  x         The target x coordinate.
     * @param[in]  y         The target y coordinate.
     * @param[in]  timeout   The timeout for the map transition operation.
     *
     * @return     An async task that completes when map transition is finished.
     */
    async::task<void> map_move(const std::string& map_name, uint16_t x, uint16_t y, std::chrono::milliseconds timeout);

    /**
     * @brief      Change the bot's level using the '/레벨바꾸기' command.
     *
     *             Executes the level change command and waits for the update_internal
     *             response to confirm the level has been changed to the target level.
     *
     * @param[in]  level   The target level to change to.
     * @param[in]  timeout The timeout for the level change operation.
     *
     * @return     An async task that completes when level change is finished.
     */
    async::task<void> change_level(uint8_t level, std::chrono::milliseconds timeout);

    /**
     * @brief      Change the bot's stats using the '/스탯바꾸기' command.
     *
     *             Executes the stats change command and waits for the update_internal
     *             response to confirm the stats have been changed to the target stats.
     *
     * @param[in]  str         The target strength value.
     * @param[in]  dex         The target dexterity value.
     * @param[in]  intelligence The target intelligence value.
     * @param[in]  timeout     The timeout for the stats change operation.
     *
     * @return     An async task that completes when stats change is finished.
     */
    async::task<void> change_stats(uint8_t str, uint8_t dex, uint8_t intelligence, std::chrono::milliseconds timeout);
    async::task<void> change_str(uint8_t str, std::chrono::milliseconds timeout);
    async::task<void> change_dex(uint8_t dex, std::chrono::milliseconds timeout);
    async::task<void> change_int(uint8_t intelligence, std::chrono::milliseconds timeout);

    /**
     * @brief      Change the bot's sex using the '/성별바꾸기' command.
     *
     * @param[in]  sex       The target sex value.
     * @param[in]  timeout   The timeout for the sex change operation.
     *
     * @return     An async task that completes when sex change is finished.
     */
    async::task<void> change_sex(SEX sex, std::chrono::milliseconds timeout);

    /**
     * @brief      Change the bot's base HP using the '/체력바꾸기' command.
     *
     * @param[in]  hp        The target base HP value.
     * @param[in]  timeout   The timeout for the base HP change operation.
     *
     * @return     An async task that completes when base HP change is finished.
     */
    async::task<void> change_base_hp(uint32_t hp, std::chrono::milliseconds timeout);

    /**
     * @brief      Change the bot's base MP using the '/마력바꾸기' command.
     *
     * @param[in]  mp        The target base MP value.
     * @param[in]  timeout   The timeout for the base MP change operation.
     *
     * @return     An async task that completes when base MP change is finished.
     */
    async::task<void> change_base_mp(uint32_t mp, std::chrono::milliseconds timeout);

    /**
     * @brief      Change the bot's current HP using the '/현재체력' command.
     *
     * @param[in]  hp        The target current HP value.
     * @param[in]  timeout   The timeout for the current HP change operation.
     *
     * @return     An async task that completes when current HP change is finished.
     */
    async::task<void> change_hp(uint32_t hp, std::chrono::milliseconds timeout);

    /**
     * @brief      Change the bot's current MP using the '/현재마력' command.
     *
     * @param[in]  mp        The target current MP value.
     * @param[in]  timeout   The timeout for the current MP change operation.
     *
     * @return     An async task that completes when current MP change is finished.
     */
    async::task<void> change_mp(uint32_t mp, std::chrono::milliseconds timeout);

    /**
     * @brief      Change the bot's facing direction.
     *
     * @param[in]  direction  The direction to face.
     * @param[in]  timeout    The timeout for the direction change operation.
     *
     * @return     An async task that completes when direction change is finished.
     */
    async::task<void> direction(DIRECTION direction, std::chrono::milliseconds timeout);

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
     * @return     An async task that completes when loot action is finished.
     */
    async::task<void> pattern_loot();

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
     * @brief      Gets a spell from the spell book by slot.
     *
     * @param[in]  slot  The inventory slot index.
     *
     * @return     An optional containing the spell if found, otherwise nullopt.
     */
    std::optional<simple_spell> get_spell(uint8_t slot) const;

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

    /**
     * @brief      Checks if bot has a specific item by name.
     *
     * @param[in]  name  The item name to search for.
     * @return     True if bot has the item, false otherwise.
     */
    bool has_item_by_name(const std::string& name) const;

    /**
     * @brief      Gets the count of a specific item in bot's inventory.
     *
     * @param[in]  name  The item name to search for.
     * @return     The item count.
     */
    uint16_t get_item_count_by_name(const std::string& name) const;

    /**
     * @brief      Gets the inventory slot index of a specific item.
     *
     * @param[in]  name  The item name to search for.
     * @return     The slot index, or 0xFF if not found.
     */
    uint8_t get_item_slot_by_name(const std::string& name) const;

    /**
     * @brief      Gets the spell slot index of a specific spell.
     *
     * @param[in]  name  The spell name to search for.
     * @return     The slot index, or 0xFF if not found.
     */
    uint8_t get_spell_slot_by_name(const std::string& name) const;

    /**
     * @brief      Remove all buffs from the bot.
     *
     *             Executes the '/버프해제' command to remove all active buffs.
     */
    void remove_buffs();

    /**
     * @brief      Send a chat message.
     *
     * @param[in]  message  The chat message to send.
     */
    void chat(const std::string& message);

    /**
     * @brief      Create items using the '/아이템생성' command.
     *
     * @param[in]  item_name  The name of the item to create.
     * @param[in]  count      The quantity of items to create.
     * @param[in]  timeout    The timeout for the operation.
     *
     * @return     An async task that completes when item creation is finished.
     */
    async::task<void> create_item(const std::string& item_name, uint32_t count, std::chrono::milliseconds timeout);

    /**
     * @brief      Change money using the '/금전바꾸기' command.
     *
     * @param[in]  amount   The amount of money to set.
     * @param[in]  timeout  The timeout for the operation.
     *
     * @return     An async task that completes when money change is finished.
     */
    async::task<void> change_money(uint32_t amount, std::chrono::milliseconds timeout);

    /**
     * @brief      Drop an item from the inventory.
     *
     * @param[in]  index    The index of the item to drop.
     * @param[in]  all      Whether to drop all items.
     * @param[in]  timeout  The timeout for the operation.
     */
    async::task<void> drop_item(uint8_t index, bool all, std::chrono::milliseconds timeout);

    /**
     * @brief      Drop money from the inventory.
     *
     * @param[in]  amount   The amount of money to drop.
     * @param[in]  timeout  The timeout for the operation.
     *
     * @return     An async task that completes when money drop is finished.
     */
    async::task<void> drop_money(uint32_t amount, std::chrono::milliseconds timeout);

    /**
     * @brief      Equip an item from the inventory.
     *
     * @param[in]  slot           The inventory slot index of the item to equip.
     * @param[in]  item_name      The name of the item to equip.
     * @param[in]  timeout        The timeout for the operation.
     *
     * @return     An async task that completes with true if operation succeeded, false otherwise.
     */
    async::task<bool> equip(uint8_t slot, const std::string& item_name, std::chrono::milliseconds timeout);

    /**
     * @brief      Unequip an item from the equipped slots.
     *
     * @param[in]  parts    The equipped parts to unequip.
     * @param[in]  timeout  The timeout for the operation.
     *
     * @return     An async task that completes with true if operation succeeded, false otherwise.
     */
    async::task<bool> unequip(EQUIPMENT_PARTS parts, std::chrono::milliseconds timeout);

    /**
     * @brief      Sleep for a specified duration.
     *
     *             This function is used to pause the bot's execution for a specified duration.
     *
     * @param[in]  timeout  The duration to sleep.
     *
     * @return     An async task that completes when sleep is finished.
     */
    async::task<void> sleep(std::chrono::milliseconds timeout);

    /**
     * @brief      Change the bot's class using the '/직업바꾸기' command.
     *
     *             Executes the class change command and waits for the update_internal
     *             response to confirm the class has been changed to the target class.
     *
     * @param[in]  class_name  The name of the class to change to.
     * @param[in]  timeout     The timeout for the operation.
     *
     * @return     An async task that completes when class change is finished.
     */
    async::task<void> change_class(const std::string& class_name, std::chrono::milliseconds timeout);

    /**
     * @brief      Spawns a monster at the specified position with custom validator
     *
     * @param[in]  bot           The bot instance to use for spawning
     * @param[in]  monster_name  Name of the monster to spawn (e.g., "다람쥐")
     * @param[in]  x             X coordinate for monster spawn
     * @param[in]  y             Y coordinate for monster spawn
     * @param[in]  validator     Custom validator function to check spawn success
     * @param[in]  timeout       The timeout for the operation
     *
     * @return     An async task that returns spawned monster information
     */
    async::task<spawned_monster_info>
    spawn_monster_with_validator(std::shared_ptr<fb::bot::game_bot>                               bot,
                                 const std::string&                                               monster_name,
                                 uint16_t                                                         x,
                                 uint16_t                                                         y,
                                 std::function<bool(const fb::protocol::game::response::update&)> validator,
                                 std::chrono::milliseconds                                        timeout);

    /**
     * @brief      Spawns a monster at the specified position with look validation
     *
     * @param[in]  monster_name   Name of the monster to spawn (e.g., "다람쥐")
     * @param[in]  x              X coordinate for monster spawn
     * @param[in]  y              Y coordinate for monster spawn
     * @param[in]  timeout        The timeout for the operation
     *
     * @return     An async task that returns spawned monster information
     */
    async::task<spawned_monster_info>
    spawn_monster(const std::string& monster_name, uint16_t x, uint16_t y, std::chrono::milliseconds timeout);

    /**
     * @brief      Spawns multiple monsters at the specified position with look validation
     *
     * @param[in]  monster_name   Name of the monster to spawn (e.g., "다람쥐")
     * @param[in]  range          Range of the monster spawn
     * @param[in]  timeout        The timeout for the operation
     *
     * @return     An async task that completes when the monsters are spawned
     */
    async::task<void> spawn_monsters_bulk(const std::string&        monster_name,
                                          uint8_t                   range,
                                          std::chrono::milliseconds timeout);

    /**
     * @brief      Spawns multiple monsters at relative positions from a bot with custom validator
     *
     * @param[in]  bot               The bot instance to use for spawning
     * @param[in]  monster_name      Name of the monster to spawn
     * @param[in]  relative_positions Vector of {x, y} relative positions from bot
     * @param[in]  validator         Custom validator function to check spawn success
     * @param[in]  timeout           The timeout for the operation
     *
     * @return     An async task that returns vector of spawned monster information
     */
    async::task<std::vector<spawned_monster_info>>
    spawn_monsters_relative_with_validator(std::shared_ptr<fb::bot::game_bot>      bot,
                                           const std::string&                      monster_name,
                                           const std::vector<std::pair<int, int>>& relative_positions,
                                           std::function<bool(const fb::protocol::game::response::update&)> validator,
                                           std::chrono::milliseconds                                        timeout);

    /**
     * @brief      Spawns multiple monsters at relative positions from a bot with look validation
     *
     * @param[in]  monster_name       Name of the monster to spawn
     * @param[in]  relative_positions Vector of {x, y} relative positions from bot
     * @param[in]  timeout            The timeout for the operation
     *
     * @return     An async task that returns vector of spawned monster information
     */
    async::task<std::vector<spawned_monster_info>>
    spawn_monsters_relative(const std::string&                      monster_name,
                            const std::vector<std::pair<int, int>>& relative_positions,
                            std::chrono::milliseconds               timeout);

    /**
     * @brief      Spawns a single monster at relative position from a bot with look validation
     *
     * @param[in]  monster_name   Name of the monster to spawn
     * @param[in]  relative_x     X coordinate relative to bot position
     * @param[in]  relative_y     Y coordinate relative to bot position
     * @param[in]  timeout        The timeout for the operation
     *
     * @return     An async task that returns spawned monster information
     */
    async::task<spawned_monster_info> spawn_monster_relative(const std::string&        monster_name,
                                                             int                       relative_x,
                                                             int                       relative_y,
                                                             std::chrono::milliseconds timeout);

    /**
     * @brief      Sets maximum HP and MP for a bot
     *
     * @param[in]  max_hp   Maximum HP value to set
     * @param[in]  max_mp   Maximum MP value to set
     * @param[in]  timeout  The timeout for the operation
     *
     * @return     An async task that returns true if HP/MP were set successfully
     */
    async::task<bool> set_max_hp_mp(int max_hp, int max_mp, std::chrono::milliseconds timeout);

    /**
     * @brief      Sets current HP and MP for a bot
     *
     * @param[in]  current_hp  Current HP value to set
     * @param[in]  current_mp  Current MP value to set
     * @param[in]  timeout     The timeout for the operation
     *
     * @return     An async task that returns true if HP/MP were set successfully
     */
    async::task<bool> set_current_hp_mp(int current_hp, int current_mp, std::chrono::milliseconds timeout);

    /**
     * @brief      Sets the bot's stats
     *
     * @param[in]  max_hp       Maximum HP value to set
     * @param[in]  max_mp       Maximum MP value to set
     * @param[in]  current_hp   Current HP value to set
     * @param[in]  current_mp   Current MP value to set
     * @param[in]  timeout      The timeout for the operation
     *
     * @return     An async task that returns true if stats were set successfully
     */
    async::task<bool> setup_bot_stats(int                       max_hp,
                                      int                       max_mp,
                                      std::optional<int>        current_hp,
                                      std::optional<int>        current_mp,
                                      std::chrono::milliseconds timeout);

    /**
     * @brief      Learns a spell for a bot
     *
     * @param[in]  spell_name  The name of the spell to learn
     * @param[in]  timeout     The timeout for the operation
     *
     * @return     An async task that returns the spell index if spell was learned successfully, 0xFF otherwise
     */
    async::task<uint8_t> learn_spell(const std::string& spell_name, std::chrono::milliseconds timeout);

    /**
     * @brief      Learns multiple spells for a bot
     *
     * @param[in]  spell_names  Vector of spell names to learn
     * @param[in]  timeout      The timeout for the operation
     *
     * @return     An async task that returns the number of successfully learned spells
     */
    async::task<size_t> learn_spells(const std::vector<std::string>& spell_names, std::chrono::milliseconds timeout);

    /**
     * @brief      Removes all spells from a bot
     *
     * @param[in]  timeout  The timeout for the operation
     *
     * @return     An async task that completes when the spells are removed
     */
    async::task<void> clear_all_spells(std::chrono::milliseconds timeout);

    /**
     * @brief      Removes all items from a map
     *
     * @param[in]  timeout  The timeout for the operation
     *
     * @return     An async task that completes when the items are removed
     */
    async::task<void> clear_all_drop_items(std::chrono::milliseconds timeout);

    /**
     * @brief      Clears all items from the inventory
     *
     * @param[in]  timeout  The timeout for the operation
     *
     * @return     An async task that completes when the items are removed
     */
    async::task<void> clear_inventory(std::chrono::milliseconds timeout);

    /**
     * @brief      Fills the inventory to capacity with a specific item
     *
     * @param[in]  name     The name of the item to fill the inventory with
     * @param[in]  timeout  The timeout for the operation
     *
     * @return     An async task that completes when the inventory is filled
     */
    async::task<void> fill_inventory(const std::string& name, std::chrono::milliseconds timeout);

    /**
     * @brief      Moves a bot back to its original position after movement spells
     *
     * @param[in]  original_position  The original position to return to
     * @param[in]  interval           The interval to move the bot back to the original position
     * @param[in]  timeout            The timeout for the operation
     *
     * @return     An async task that completes when the bot has returned to original position
     */
    async::task<void> move_bot_back_to_position(const fb::model::point<uint16_t>& original_position,
                                                std::chrono::milliseconds         interval,
                                                std::chrono::milliseconds         timeout);

    async::task<void> reverse_condition(const std::vector<fb::model::dsl>& conditions,
                                        std::chrono::milliseconds          timeout);

    async::task<void> apply_condition(const std::vector<fb::model::dsl>& conditions, std::chrono::milliseconds timeout);
};

} // namespace fb::bot

#endif