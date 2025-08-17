#ifndef __CHARACTER_H__
#define __CHARACTER_H__

/**
 * @file    character.h
 * @brief   Player character system for the FB 2D MMORPG game server
 * @author  FB Development Team
 *
 * @details This file implements the core player character system that handles all
 *          aspects of player avatars in the FB 2D MMORPG game server. Characters
 *          represent players in the game world and manage their state, equipment,
 *          skills, social relationships, and interactions.
 *
 *          Key features:
 *          - Complete character state management (stats, level, experience)
 *          - Equipment and inventory systems with item management
 *          - Spell casting and buff/debuff systems
 *          - Social systems (groups, clans, whispers, mail)
 *          - Achievement and progression tracking
 *          - Thread-safe operations with async/await patterns
 *          - Lua scripting integration for game logic
 *          - Real-time synchronization across server instances
 *          - Database persistence and character data management
 *          - Map movement and position tracking
 *
 * @note    Characters inherit from life and object classes, providing a complete
 *          game entity with combat, movement, and interaction capabilities.
 */

#include <fb/game/life.h>
#include <algorithm>
#include <fb/game/dialog.h>
#include <fb/game/character.h>
#include <fb/locker.h>
#include <fb/socket.h>
#include <iostream>
#include <fb/game/mob.h>
#include <sstream>
#include <fb/game/trade.h>
#include <fb/game/achievement.h>
#include <fb/game/bulletin.h>
#include <fb/game/stat.h>
#include <fb/game/quest.h>

namespace fb::game {

/**
 * @brief      Forward declaration of the map class.
 */
class map;

/**
 * @brief      Represents a player character in the game world.
 *
 *             This class extends the life class to provide complete player character functionality.
 *             It manages all aspects of a player character including stats, inventory, equipment,
 *             spells, social features (groups, clans), and network communication. The character
 *             serves as the primary interface between the player client and the game world.
 *
 *             Key features:
 *             - Complete character progression system (level, experience, stats)
 *             - Inventory and equipment management
 *             - Spell and skill system integration
 *             - Group membership
 *             - Real-time network communication with client
 *             - Lua scripting integration for game logic
 *             - Achievement and quest system
 *             - Trading and economic interactions
 *             - PvP and PvE combat mechanics
 */
class character : public life
{
    friend class group;

public:
    using object::map;

public:
    class container;

private:
    // Type aliases for better readability
    using socket_ptr_t      = std::weak_ptr<fb::socket<character>>;
    using mob_vector_t      = std::vector<std::shared_ptr<fb::game::mob>>;
    using achievement_map_t = std::map<uint32_t, std::unique_ptr<achievement>>;

public:
    LUA_PROTOTYPE

public:
    struct listener_t;

private:
    uint32_t                _id;
    socket_ptr_t            _socket;
    std::string             _name;
    ROLE                    _role;
    std::string             _pw;
    std::optional<uint32_t> _birthday;
    fb::model::datetime     _updated_date;
    uint16_t                _look        = 0;
    uint8_t                 _color       = 0;
    std::optional<uint8_t>  _armor_color = 0;
    stat_value<uint32_t>    _max_hp, _max_mp;
    stat_value<uint8_t>     _str, _dex, _int;
    stat_value<int32_t>     _phydef, _magdef;
    stat_value<int32_t>     _dam, _hit;
    uint32_t                _experience   = 0;
    uint8_t                 _regenerative = 0; // 재생력
    NATION                  _nation       = NATION::GOGURYEO;
    CREATURE                _creature     = CREATURE::DRAGON;
    SEX                     _sex          = SEX::MAN;
    STATE                   _state        = STATE::NORMAL;
    uint8_t                 _level        = 1;
    CLASS                   _class        = CLASS::NONE;
    uint8_t                 _promotion    = 0;
    uint32_t                _money        = 0;
    std::optional<uint16_t> _disguise     = 0;
    std::string             _title;
    std::optional<uint32_t> _group_id      = std::nullopt;
    std::optional<uint32_t> _clan_id       = std::nullopt;
    uint16_t                _unread_mail   = 0;
    uint16_t                _weapon_damage = 0;
    bool                    _detect        = false;
    mob_vector_t            _spawned_mobs  = {};
    fb::model::datetime     _last_spell_cast;
    bool                    _super_hide        = false;
    uint8_t                 _spell_cast_count  = 0;
    bool                    _options[0x0B + 1] = {
        1,
    };

public:
    fb::game::trade   trade;
    fb::game::items   items;
    fb::game::quests  quests;
    fb::lua::context* dialog = nullptr;
    achievement_map_t achievements; // order required
    listener_t&       listener;

private:
    using object::based;

public:
    /**
     * @brief      Constructs a new character instance with game context and network socket.
     *
     *             Initializes a new character with the provided game context and network socket.
     *             The socket reference is used to obtain a weak_ptr internally to prevent
     *             circular references and ensure proper cleanup when the socket is destroyed.
     *             Sets up default values for all character attributes, initializes the
     *             inventory and equipment systems, and prepares the character for game world
     *             interaction.
     *
     * @param      server   The game server that manages this character.
     * @param      socket   Reference to the network socket for client communication.
     */
    character(fb::game::server& server, fb::socket<character>& socket);

    /**
     * @brief      Destroys the character and cleans up all associated resources.
     *
     *             Ensures proper cleanup of inventory items, spell effects, group/clan
     *             memberships, and any other resources associated with the character.
     */
    ~character();

public:
    void on_init() override final;

private:
    /**
     * @brief      Calculates the experience points limited by level cap and progression rules.
     *
     *             Applies level-based experience limitations to prevent excessive experience
     *             gain that would break game balance. The limitation is based on the character's
     *             current level and configured experience progression curves.
     *
     * @param[in]  exp   The raw experience points to be limited.
     *
     * @return     The experience points after applying level-based limitations.
     */
    uint32_t limited_exp(uint32_t exp) const;

public:
    /**
     * @brief      Sends a binary stream to the character's client with optional encryption.
     *
     *             Transmits raw binary data to the character's client through the network socket.
     *             Supports optional encryption and protocol wrapping for secure communication.
     *             This is the low-level send method used by higher-level protocol methods.
     *
     * @param[in]  stream   The binary stream containing the data to send.
     * @param[in]  encrypt  Whether to encrypt the data before transmission.
     * @param[in]  wrap     Whether to wrap the data with protocol headers.
     *
     * @return     An async task that completes with the number of bytes sent.
     */
    async::task<size_t> send(const fb::stream& stream, bool encrypt = true, bool wrap = true) override final;

    /**
     * @brief      Sends a protocol response to the character's client.
     *
     * @param[in]  response  The protocol response to send
     * @param[in]  encrypt   Whether to encrypt the response
     * @param[in]  wrap      Whether to wrap the response in protocol headers
     *
     * @return     Number of bytes sent to the client
     */
    async::task<size_t> send(const fb::protocol::header& response,
                             bool                        encrypt = true,
                             bool                        wrap    = true) override final;

    /**
     * @brief      Gets the object type (always CHARACTER).
     *
     * @return     The object type.
     */
    OBJECT_TYPE what() const override final;

    /**
     * @brief      Moves the character to a different map at a specific position.
     *
     * @param      map           The target map to move to
     * @param[in]  position      The position on the target map
     * @param[in]  destroy_type  How to handle the character when leaving current map
     * @param[in]  notify        Whether to notify other objects about the move
     *
     * @return     True if the map change was successful, false otherwise
     */
    [[nodiscard]] async::task<bool> map(std::shared_ptr<fb::game::map> map,
                                        const fb::model::point16_t&    position,
                                        DESTROY_TYPE                   destroy_type = DESTROY_TYPE::DEFAULT,
                                        bool                           notify       = true) override final;

public:
    /**
     * @brief      Checks if the character has been fully initialized.
     *
     * @return     True if the character is initialized, false otherwise
     */
    bool inited() const;

    /**
     * @brief      Gets the character's socket.
     *
     * @return     The character's socket
     */
    fb::socket<character>* socket() const;

    /**
     * @brief      Gets the character's unique database ID.
     *
     * @return     The character's database ID
     */
    uint32_t id() const;

    /**
     * @brief      Sets the character's unique database ID.
     *
     * @param[in]  id    The new character database ID
     */
    void id(uint32_t id);

    /**
     * @brief      Gets the character's role (player, admin, etc.).
     *
     * @return     The character's current role
     */
    ROLE role() const;

    /**
     * @brief      Sets the character's role.
     *
     * @param[in]  value  The new role to assign
     */
    void role(ROLE value);

    /**
     * @brief      Checks if the character is currently transferring between maps.
     *
     * @return     True if transferring, false otherwise
     */
    bool transferring() const;

    /**
     * @brief      Performs an attack action with specified duration.
     *
     * @param[in]  duration  The attack duration type
     */
    async::task<void> attack(DURATION duration = DURATION::ATTACK) override final;

    /**
     * @brief      Calculates auto-attack damage based on target size.
     *
     * @param[in]  size  The target mob size (small/large affects damage)
     *
     * @return     The calculated damage value
     */
    uint32_t auto_attack_damage(MOB_SIZE size) const override final;

    /**
     * @brief      Applies damage to the character with equipment durability loss.
     *
     * @param[in]  value     The base damage amount
     * @param      from      The source object causing damage
     * @param[in]  critical  Whether this is a critical hit
     *
     * @return     The actual damage dealt after calculations
     */
    uint32_t damage(uint32_t                          value,
                    std::shared_ptr<fb::game::object> from     = nullptr,
                    bool                              critical = false) override final;

    /**
     * @brief      Performs a character action with visual and audio effects.
     *
     * @param[in]  action    The action type to perform
     * @param[in]  duration  The duration of the action
     * @param[in]  sound     The sound effect ID to play
     */
    void action(ACTION action, DURATION duration, uint8_t sound = 0x00) override final;

    /**
     * @brief      Gets the character's name.
     *
     * @return     Reference to the character's name string
     */
    const std::string& name() const override final;

    /**
     * @brief      Sets the character's name.
     *
     * @param[in]  value  The new character name
     */
    void name(const std::string& value);

    /**
     * @brief      Sets the character's password.
     *
     * @param[in]  value  The new password
     */
    void pw(const std::string& value);

    /**
     * @brief      Gets the character's birthday as a timestamp.
     *
     * @return     Optional birthday timestamp, or nullopt if not set
     */
    const std::optional<uint32_t>& birthday() const;

    /**
     * @brief      Sets the character's birthday timestamp.
     *
     * @param[in]  value  The birthday timestamp to set
     */
    void birthday(const std::optional<uint32_t>& value);

    /**
     * @brief      Gets the character's last update timestamp.
     *
     * @return     Reference to the last updated date and time
     */
    const fb::model::datetime& updated_date() const;

    /**
     * @brief      Sets the character's last update timestamp.
     *
     * @param[in]  value  The new update timestamp to set
     */
    void updated_date(const fb::model::datetime& value);

    /**
     * @brief      Gets the character's visual appearance ID.
     *
     * @return     The character's sprite/appearance ID for rendering
     */
    uint16_t look() const override final;

    /**
     * @brief      Sets the character's visual appearance ID.
     *
     * @param[in]  value  The new appearance ID to set
     */
    void look(uint16_t value);

    /**
     * @brief      Gets the character's hair/skin color.
     *
     * @return     The character's color value for rendering
     */
    uint8_t color() const override final;

    /**
     * @brief      Sets the character's hair/skin color.
     *
     * @param[in]  value  The new color value to set
     */
    void color(uint8_t value);

    /**
     * @brief      Gets the character's armor color override.
     *
     * @return     Optional armor color, or nullopt if using default armor color
     */
    std::optional<uint8_t> armor_color() const;

    /**
     * @brief      Sets the character's armor color override.
     *
     * @param[in]  value  The armor color to set, or nullopt to use default
     */
    void armor_color(std::optional<uint8_t> value);

    /**
     * @brief      Gets the character's current effective armor color.
     *
     * @return     The armor color currently being displayed (override or default)
     */
    uint8_t current_armor_color() const;

    /**
     * @brief      Gets the character's current disguise appearance.
     *
     * @return     Optional disguise appearance ID, or nullopt if not disguised
     */
    std::optional<uint16_t> disguise() const;

    /**
     * @brief      Sets the character's disguise appearance.
     *
     * @param[in]  value  The disguise appearance ID to apply
     */
    void disguise(uint16_t value);

    /**
     * @brief      Removes the character's disguise and returns to normal appearance.
     */
    void undisguise();

    /**
     * @brief      Gets the character's nation affiliation.
     *
     * @return     The character's current nation (Koguryo, Buyo, etc.)
     */
    NATION nation() const;

    /**
     * @brief      Sets the character's nation affiliation.
     *
     * @param[in]  value  The new nation to assign to the character
     *
     * @return     True if the nation was successfully changed, false otherwise
     */
    bool nation(NATION value);

    /**
     * @brief      Gets the character's creature type.
     *
     * @return     The character's creature type (human, elf, etc.)
     */
    CREATURE creature() const;

    /**
     * @brief      Sets the character's creature type.
     *
     * @param[in]  value  The new creature type to assign
     *
     * @return     True if the creature type was successfully changed, false otherwise
     */
    bool creature(CREATURE value);

    /**
     * @brief      Gets the character's current level.
     *
     * @return     The character's level (1-99)
     */
    uint8_t level() const;

    /**
     * @brief      Sets the character's level.
     *
     * @param[in]  value  The new level to set (1-99)
     */
    void level(uint8_t value);

    /**
     * @brief      Attempts to level up the character.
     *
     * @return     True if the character successfully leveled up, false otherwise
     */
    bool level_up();

    /**
     * @brief      Checks if the character is at maximum level.
     *
     * @return     True if the character is at max level, false otherwise
     */
    bool max_level() const;

    /**
     * @brief      Gets the character's gender.
     *
     * @return     The character's gender (male or female)
     */
    SEX sex() const;

    /**
     * @brief      Sets the character's gender.
     *
     * @param[in]  value  The gender to set (male or female)
     */
    void sex(SEX value);

    /**
     * @brief      Gets the character's current state.
     *
     * @return     The character's current state (normal, dead, sleeping, etc.)
     */
    STATE state() const;

    /**
     * @brief      Gets the character's state as seen by a specific observer.
     *
     * @param[in]  to    The observer object checking this character's state
     *
     * @return     The character's state as perceived by the observer
     */
    STATE state_to(const fb::game::object& to) const;

    /**
     * @brief      Sets the character's state.
     *
     * @param[in]  value  The new state to set (normal, dead, sleeping, etc.)
     */
    void state(STATE value);

    /**
     * @brief      Gets the character's class.
     *
     * @return     The character's class (warrior, rogue, wizard, priest)
     */
    CLASS cls() const;

    /**
     * @brief      Sets the character's class.
     *
     * @param[in]  value  The new class to assign
     */
    void cls(CLASS value);

    /**
     * @brief      Gets the character's promotion level.
     *
     * @return     The character's promotion level (0-4)
     */
    uint8_t promotion() const;

    /**
     * @brief      Sets the character's promotion level.
     *
     * @param[in]  value  The new promotion level to set (0-4)
     */
    void promotion(uint8_t value);

    /**
     * @brief      Gets the character's current experience points.
     *
     * @return     The character's current experience points
     */
    uint32_t exp() const;

    /**
     * @brief      Sets the character's experience points.
     *
     * @param[in]  value  The new experience points to set
     */
    void exp(uint32_t value);

    /**
     * @brief      Adds an exponent.
     *
     * @param[in]  value   The amount of experience to add
     * @param[in]  limit   Whether to apply level cap limits
     * @param[in]  notify  Whether to notify the client of the change
     *
     * @return     The actual experience points added
     */
    uint32_t add_exp(uint32_t value, bool limit = false, bool notify = false);

    /**
     * @brief      Reduces the exponent.
     *
     * @param[in]  value  The amount of experience to reduce
     *
     * @return     The actual experience points reduced
     */
    uint32_t reduce_exp(uint32_t value);

    /**
     * @brief      Gets the experience points needed to reach the next level.
     *
     * @return     The remaining experience points needed for next level
     */
    uint32_t experience_remained() const;

    /**
     * @brief      Gets the experience progress percentage for the current level.
     *
     * @return     The experience percentage (0.0 to 1.0) for current level
     */
    float experience_percent() const;

    /**
     * @brief      Gets the character's current money amount.
     *
     * @return     The character's current money in inventory
     */
    uint32_t money() const;

    /**
     * @brief      Sets the character's money amount.
     *
     * @param[in]  value  The new money amount to set
     */
    void money(uint32_t value);

    /**
     * @brief      Adds money to the character's inventory.
     *
     * @param[in]  value  The amount of money to add
     *
     * @return     The actual amount of money added
     */
    uint32_t money_add(uint32_t value);

    /**
     * @brief      Reduces money from the character's inventory.
     *
     * @param[in]  value  The amount of money to reduce
     */
    void money_reduce(uint32_t value);

    /**
     * @brief      Drops money as a cash item on the ground.
     *
     * @param[in]  value  The amount of money to drop
     *
     * @return     Pointer to the created cash item, or nullptr if failed
     */
    fb::game::cash* money_drop(uint32_t value);

    /**
     * @brief      Gets the character's regeneration rate.
     *
     * @return     The character's health/mana regeneration rate
     */
    uint32_t regenerative() const;

    /**
     * @brief      Sets the character's regeneration rate.
     *
     * @param[in]  value  The new regeneration rate to set
     */
    void regenerative(uint8_t value);

    /**
     * @brief      Gets the value of a character option setting.
     *
     * @param[in]  key   The option key to check
     *
     * @return     True if the option is enabled, false otherwise
     */
    bool option(OPTION key) const;

    /**
     * @brief      Sets a character option setting.
     *
     * @param[in]  key     The option key to set
     * @param[in]  value   The new option value
     * @param[in]  notify  Whether to notify the client of the change
     */
    void option(OPTION key, bool value, bool notify = true);

    /**
     * @brief      Toggles a character option setting.
     *
     * @param[in]  key     The option key to toggle
     * @param[in]  notify  Whether to notify the client of the change
     *
     * @return     The new value after toggling
     */
    bool option_toggle(OPTION key, bool notify = true);

    /**
     * @brief      Updates and synchronizes character options with the client.
     */
    void update_option();

    /**
     * @brief      Updates the character's map view with the entire map.
     *
     * @param[in]  map   The map to update the character's view with
     */
    void update_map(const fb::game::map& map) override final;

    /**
     * @brief      Updates the character's current map view.
     */
    void update_map();

    /**
     * @brief      Updates a specific region of the character's map view.
     *
     * @param[in]  map       The map to update from
     * @param[in]  begin     The position of the region to update
     * @param[in]  size      The size of the region to update
     * @param[in]  crc       The CRC value of the map update
     */
    void update_map(const fb::game::map&        map,
                    const fb::model::point16_t& begin,
                    const fb::model::size8_t&   size,
                    uint16_t                    crc = 0);

    /**
     * @brief      Updates the character's background music.
     *
     * @param[in]  bgm     The background music ID to play
     * @param[in]  volume  The volume level for the music (0-255)
     */
    void update_bgm(uint16_t bgm, uint8_t volume) override final;

    /**
     * @brief      Updates and synchronizes the character's buff effects.
     */
    void update_buff();

    /**
     * @brief      Updates the character's internal state and data.
     */
    void update_internal();

    /**
     * @brief      Updates the character's time-based effects and states.
     *
     * @param[in]  hours  The current game time in hours
     */
    void update_time(uint16_t hours);

    /**
     * @brief      Initializes the object.
     */
    void init();

    /**
     * @brief      Updates the character's position and notifies nearby objects.
     */
    void update_position() override final;

    /**
     * @brief      Gets the character's title.
     *
     * @return     Reference to the character's current title string
     */
    const std::string& title() const;

    /**
     * @brief      Sets the character's title.
     *
     * @param[in]  value  The new title to assign to the character
     */
    void title(const std::string& value);

    /**
     * @brief      Gets the character's group ID (read-only).
     *
     *             Returns the unique identifier of the group this character belongs to.
     *             If the character is not in any group, returns std::nullopt.
     *
     * @return     Optional group ID if character is in a group, std::nullopt otherwise
     */
    const std::optional<uint32_t>& group_id() const;

    /**
     * @brief      Gets the character's group ID (modifiable).
     *
     *             Returns a modifiable reference to the group ID. This allows
     *             direct manipulation of the group membership status.
     *
     * @return     Reference to the character's optional group ID
     */
    std::optional<uint32_t>& group_id();

    /**
     * @brief      Sets the character's group ID.
     *
     *             Assigns the character to a specific group by setting the group ID.
     *             This establishes the character's membership in the specified group.
     *
     * @param[in]  gid  The group ID to assign to the character
     */
    void group_id(uint32_t gid);

    /**
     * @brief      Removes the character from their current group.
     *
     *             Resets the character's group membership by clearing the group ID.
     *             After calling this method, the character will not belong to any group.
     */
    void group_reset();

    /**
     * @brief      Gets the character's clan id (modifiable).
     *
     * @return     Reference to the character's clan id
     */
    const std::optional<uint32_t>& clan_id() const;

    /**
     * @brief      Sets the character's clan.
     *
     * @param      value  The clan id to assign to the character
     */
    void clan_id(std::optional<uint32_t> value);

    /**
     * @brief      Resets the character's clan.
     */
    void clan_reset();

    /**
     * @brief      Asserts that the character is in a specific state.
     *
     * @param[in]  value  The required state to check
     */
    void assert_state(STATE value) const;

    /**
     * @brief      Asserts that the character is in one of the specified states.
     *
     * @param[in]  values  The list of acceptable states
     */
    void assert_state(const std::vector<STATE>& values) const;

    /**
     * @brief      Moves the character to a new position.
     *
     * @param[in]  before  The previous position before the move
     *
     * @return     True if the move was successful, false otherwise
     */
    bool move(const fb::model::point16_t& before);

    /**
     * @brief      Moves the character in a specific direction.
     *
     * @param[in]  direction  The direction to move
     * @param[in]  before     The previous position before the move
     *
     * @return     True if the move was successful, false otherwise
     */
    bool move(DIRECTION direction, const fb::model::point16_t& before);

    /**
     * @brief      Makes the character ride a specific mount.
     *
     * @param      horse  The mob to ride as a mount
     */
    void ride(mob& horse);

    /**
     * @brief      Makes the character ride their current mount.
     */
    void ride();

    /**
     * @brief      Makes the character dismount from their current mount.
     */
    void unride();

    /**
     * @brief      Checks if the character is alive.
     *
     * @return     True if the character is alive, false if dead
     */
    bool alive() const;

    /**
     * @brief      Checks if the character meets the specified conditions.
     *
     * @param[in]  conditions  The list of conditions to evaluate
     *
     * @return     True if all conditions are met, false otherwise
     */
    bool condition(const std::vector<fb::model::dsl>& conditions) const override final;

    /**
     * @brief      Sends a message to the character.
     *
     * @param[in]  message  The message text to send
     * @param[in]  type     The type of message (default: STATE)
     */
    void message(const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE);

    /**
     * @brief      Gets the thread this character belongs to.
     *
     * @return     Pointer to the thread managing this character
     */
    fb::thread* thread() const override final;

    /**
     * @brief      Sets the thread that this character belongs to.
     *
     * @param[in]  value  Pointer to the thread managing this character
     */
    void thread(fb::thread* value);

    /**
     * @brief      Asserts that the current thread is the correct thread for this character.
     */
    void assert_thread() const override final;

    /**
     * @brief      Updates the given value.
     *
     * @param[in]  value  The value
     */
    void update(STATE_LEVEL value = STATE_LEVEL::LEVEL_MIN) override final;

    /**
     * @brief      Gets the number of unread mail messages.
     *
     * @return     The count of unread mail messages
     */
    uint16_t unread_mail() const;

    /**
     * @brief      Sets the number of unread mail messages.
     *
     * @param[in]  value  The new unread mail count
     */
    void unread_mail(uint16_t value);

    /**
     * @brief      Shows another character's information to this character.
     *
     * @param[in]  ch    The character to browse/inspect
     */
    void browse_ch(const character& ch);

    /**
     * @brief      Shows an item tooltip to the character.
     *
     * @param[in]  iteem     The item to show tooltip for
     * @param[in]  position  The position/slot of the item
     */
    void item_tooltip(const item& iteem, uint16_t position);

    /**
     * @brief      Shows the user list.
     */
    void show_user_list();

    /**
     * @brief      Shows the bulletin.
     */
    void show_bulletin();

    /**
     * @brief      Shows the bulletin.
     *
     * @param[in]  section   The bulletin section to display
     * @param[in]  articles  The list of articles in the section
     * @param[in]  flag      The button enable flags for the bulletin
     */
    void show_bulletin(const fb::model::bulletin&          section,
                       const std::list<bulletin::article>& articles,
                       BULLETIN_BUTTON_ENABLE              flag);

    /**
     * @brief      Shows the bulletin.
     *
     * @param[in]  article  The specific article to display
     * @param[in]  flag     The button enable flags for the article view
     */
    void show_bulletin(const bulletin::article& article, BULLETIN_BUTTON_ENABLE flag);

    /**
     * @brief      Shows the mail box.
     *
     * @param[in]  mails  The list of mail summaries to display
     * @param[in]  flag   The button enable flags for the mail box
     */
    void show_mail_box(const std::vector<fb::protocol::internal::MailSummary>& mails, MAIL_BUTTON_ENABLE flag);

    /**
     * @brief      Shows the mail box.
     *
     * @param[in]  mail  The specific mail to display
     * @param[in]  flag  The button enable flags for the mail view
     */
    void show_mail_box(const fb::protocol::internal::Mail& mail, MAIL_BUTTON_ENABLE flag);

    /**
     * @brief      Shows the bulletin message.
     *
     * @param[in]  message  The message text to display
     * @param[in]  success  Whether the operation was successful
     * @param[in]  mail     Whether this is a mail-related message
     */
    void show_bulletin_message(const std::string& message, bool success, bool mail);

    /**
     * @brief      Shows the world map.
     *
     * @param[in]  id     The world map identifier
     * @param[in]  index  The map index to highlight or focus on
     */
    void show_world_map(uint32_t id, uint16_t index);

    /**
     * @brief      Sets a timer for the character.
     *
     * @param[in]  time  The timer duration in seconds
     * @param[in]  type  The type of timer to set
     */
    void timer(uint32_t time, TIMER_TYPE type);

    /**
     * @brief      Sets the weather effect for the character.
     *
     * @param[in]  weather  The weather type to apply
     */
    void weather(WEATHER_TYPE weather);

    /**
     * @brief      Sets the brightness level for the character's view.
     *
     * @param[in]  value  The brightness level (0-255)
     */
    void bright(uint8_t value);

    /**
     * @brief      Updates the character's unique identifier.
     */
    void update_id() override final;

    /**
     * @brief      Sets the character's weapon damage modifier.
     *
     * @param[in]  value  The weapon damage value to set
     */
    void weapon_damage(uint16_t value);

    /**
     * @brief      Gets the character's current weapon damage modifier.
     *
     * @return     The current weapon damage value
     */
    uint16_t weapon_damage() const;

    /**
     * @brief      Sets the character's detect ability.
     *
     * @param[in]  value  True to enable detect, false to disable
     */
    void detect(bool value);

    /**
     * @brief      Checks if the character has detect ability enabled.
     *
     * @return     True if detect is enabled, false otherwise
     */
    bool detect() const;

    /**
     * @brief      Spawns a mob for this character.
     *
     * @param[in]  model     The mob model to spawn
     * @param[in]  position  The position where to spawn the mob
     * @param[in]  owned     Whether the mob is owned by this character
     * @param[in]  notify    Whether to notify the character about the mob
     *
     * @return     Pointer to the spawned mob, or nullptr if failed
     */
    std::shared_ptr<fb::game::mob>
    spawn_mob(const fb::model::mob& model, const fb::model::point16_t& position, bool owned = true, bool notify = true);

    /**
     * @brief      Gets the list of mobs spawned by this character.
     *
     * @return     Const reference to the vector of spawned mobs
     */
    const std::vector<std::shared_ptr<fb::game::mob>>& spawned_mobs() const;

    /**
     * @brief      Detaches the spawned mob.
     *
     * @param      mob   The mob to detach from this character
     *
     * @return     True if the mob was successfully detached, false otherwise
     */
    bool detach_spawned_mob(fb::game::mob& mob);

    /**
     * @brief      Applies death penalty to the character.
     *
     * @return     Async task that completes when penalty is applied
     */
    async::task<void> death_penalty();

    /**
     * @brief      Gets a quest by ID.
     *
     * @param[in]  id  The quest ID
     *
     * @return     Pointer to the quest, or nullptr if not found
     */
    std::shared_ptr<fb::game::quest> quest(uint32_t id) const;

    /**
     * @brief      Starts a quest for the character.
     *
     * @param[in]  id  The quest ID to start
     *
     * @return     True if the quest was started successfully, false otherwise
     */
    bool start_quest(uint32_t id);

    /**
     * @brief      Removes a quest from the character.
     *
     * @param[in]  id  The quest ID to remove
     *
     * @return     True if the quest was removed successfully, false otherwise
     */
    bool remove_quest(uint32_t id);

    /**
     * @brief      Rewards the character with the specified items and money.
     *
     * @param[in]  reward  The reward items and money to apply
     *
     * @return     True if the reward was applied successfully, false otherwise
     */
    bool reward(const std::vector<fb::model::dsl>& reward);

    /**
     * @brief      Returns a protocol representation of the object.
     *
     * @return     Protocol representation of the object.
     */
    fb::protocol::internal::Character to_protocol() const;

#pragma region stat

public:
    /**
     * @brief      Gets the character's base HP value.
     *
     * @return     The character's base health points
     */
    uint32_t base_hp() const override final;

    /**
     * @brief      Gets the character's current HP buff modifier.
     *
     * @return     The current HP buff value
     */
    uint32_t buff_hp() const override final;

    /**
     * @brief      Gets the character's maximum HP (base + buffs).
     *
     * @return     The character's maximum health points
     */
    uint32_t maxhp() const override final;

    /**
     * @brief      Sets the character's base HP value.
     *
     * @param[in]  value  The new base HP value to set
     */
    void base_hp(uint32_t value);

    /**
     * @brief      Sets the character's HP buff modifier.
     *
     * @param[in]  value  The HP buff value to apply
     */
    void buff_hp(uint32_t value) override final;

    /**
     * @brief      Gets the character's base MP value.
     *
     * @return     The character's base mana points
     */
    uint32_t base_mp() const override final;

    /**
     * @brief      Gets the character's current MP buff modifier.
     *
     * @return     The current MP buff value
     */
    uint32_t buff_mp() const override final;

    /**
     * @brief      Gets the character's maximum MP (base + buffs).
     *
     * @return     The character's maximum mana points
     */
    uint32_t maxmp() const override final;

    /**
     * @brief      Sets the character's base MP value.
     *
     * @param[in]  value  The new base MP value to set
     */
    void base_mp(uint32_t value);

    /**
     * @brief      Sets the character's MP buff modifier.
     *
     * @param[in]  value  The MP buff value to apply
     */
    void buff_mp(uint32_t value) override final;

    /**
     * @brief      Gets the character's base strength value.
     *
     * @return     The character's base strength stat
     */
    uint8_t base_str() const override final;

    /**
     * @brief      Gets the character's current strength buff modifier.
     *
     * @return     The current strength buff value
     */
    uint8_t buff_str() const override final;

    /**
     * @brief      Gets the character's total strength (base + buffs).
     *
     * @return     The character's effective strength value
     */
    uint8_t str() const override final;

    /**
     * @brief      Sets the character's base strength value.
     *
     * @param[in]  value  The new base strength value to set
     */
    void base_str(uint8_t value);

    /**
     * @brief      Sets the character's strength buff modifier.
     *
     * @param[in]  value  The strength buff value to apply
     */
    void buff_str(uint8_t value) override final;

    /**
     * @brief      Gets the character's base dexterity value.
     *
     * @return     The character's base dexterity stat
     */
    uint8_t base_dex() const override final;

    /**
     * @brief      Gets the character's current dexterity buff modifier.
     *
     * @return     The current dexterity buff value
     */
    uint8_t buff_dex() const override final;

    /**
     * @brief      Gets the character's total dexterity (base + buffs).
     *
     * @return     The character's effective dexterity value
     */
    uint8_t dex() const override final;

    /**
     * @brief      Sets the character's base dexterity value.
     *
     * @param[in]  value  The new base dexterity value to set
     */
    void base_dex(uint8_t value);

    /**
     * @brief      Sets the character's dexterity buff modifier.
     *
     * @param[in]  value  The dexterity buff value to apply
     */
    void buff_dex(uint8_t value) override final;

    /**
     * @brief      Gets the character's base intelligence value.
     *
     * @return     The character's base intelligence stat
     */
    uint8_t base_int() const override final;

    /**
     * @brief      Gets the character's current intelligence buff modifier.
     *
     * @return     The current intelligence buff value
     */
    uint8_t buff_int() const override final;

    /**
     * @brief      Gets the character's total intelligence (base + buffs).
     *
     * @return     The character's effective intelligence value
     */
    uint8_t intelligence() const override final;

    /**
     * @brief      Sets the character's base intelligence value.
     *
     * @param[in]  value  The new base intelligence value to set
     */
    void base_int(uint8_t value);

    /**
     * @brief      Sets the character's intelligence buff modifier.
     *
     * @param[in]  value  The intelligence buff value to apply
     */
    void buff_int(uint8_t value) override final;

    /**
     * @brief      Gets the character's base physical defense value.
     *
     * @return     The character's base physical defense stat
     */
    int8_t base_phydef() const override final;

    /**
     * @brief      Gets the character's current physical defense buff modifier.
     *
     * @return     The current physical defense buff value
     */
    int8_t buff_phydef() const override final;

    /**
     * @brief      Gets the character's total physical defense (base + buffs).
     *
     * @return     The character's effective physical defense value
     */
    int8_t phydef() const override final;

    /**
     * @brief      Sets the character's base physical defense value.
     *
     * @param[in]  value  The new base physical defense value to set
     */
    void base_phydef(int8_t value);

    /**
     * @brief      Sets the character's physical defense buff modifier.
     *
     * @param[in]  value  The physical defense buff value to apply
     */
    void buff_phydef(int8_t value) override final;

    /**
     * @brief      Gets the character's base magical defense value.
     *
     * @return     The character's base magical defense stat
     */
    int8_t base_magdef() const override final;

    /**
     * @brief      Gets the character's current magical defense buff modifier.
     *
     * @return     The current magical defense buff value
     */
    int8_t buff_magdef() const override final;

    /**
     * @brief      Gets the character's total magical defense (base + buffs).
     *
     * @return     The character's effective magical defense value
     */
    int8_t magdef() const override final;

    /**
     * @brief      Sets the character's base magical defense value.
     *
     * @param[in]  value  The new base magical defense value to set
     */
    void base_magdef(int8_t value);

    /**
     * @brief      Sets the character's magical defense buff modifier.
     *
     * @param[in]  value  The magical defense buff value to apply
     */
    void buff_magdef(int8_t value) override final;

    /**
     * @brief      Gets the character's base damage value.
     *
     * @return     The character's base damage stat
     */
    uint8_t base_dam() const override final;

    /**
     * @brief      Gets the character's current damage buff modifier.
     *
     * @return     The current damage buff value
     */
    uint8_t buff_dam() const override final;

    /**
     * @brief      Gets the character's total damage (base + buffs).
     *
     * @return     The character's effective damage value
     */
    uint8_t dam() const override final;

    /**
     * @brief      Sets the character's base damage value.
     *
     * @param[in]  value  The new base damage value to set
     */
    void base_dam(uint8_t value);

    /**
     * @brief      Sets the character's damage buff modifier.
     *
     * @param[in]  value  The damage buff value to apply
     */
    void buff_dam(uint8_t value) override final;

    /**
     * @brief      Gets the character's base hit value.
     *
     * @return     The character's base hit stat
     */
    uint8_t base_hit() const override final;

    /**
     * @brief      Gets the character's current hit buff modifier.
     *
     * @return     The current hit buff value
     */
    uint8_t buff_hit() const override final;

    /**
     * @brief      Gets the character's total hit (base + buffs).
     *
     * @return     The character's effective hit value
     */
    uint8_t hit() const override final;

    /**
     * @brief      Sets the character's base hit value.
     *
     * @param[in]  value  The new base hit value to set
     */
    void base_hit(uint8_t value);

    /**
     * @brief      Sets the character's hit buff modifier.
     *
     * @param[in]  value  The hit buff value to apply
     */
    void buff_hit(uint8_t value) override final;

    /**
     * @brief      Checks if the character is in super hide mode (invisible to all).
     *
     * @return     True if the character is super hidden, false otherwise
     */
    bool super_hide() const override final;

    /**
     * @brief      Sets the character's super hide mode.
     *
     * @param[in]  enabled  True to enable super hide, false to disable
     */
    void super_hide(bool enabled);

    /**
     * @brief      Checks if this character is hidden from a specific target.
     *
     * @param[in]  target  The target object to check visibility against
     *
     * @return     True if this character is hidden from the target, false otherwise
     */
    bool hidden(const fb::game::object& target) const override final;

    /**
     * @brief      Checks if the character is hidden from a specific role.
     *
     * @param[in]  role  The role to check visibility against
     *
     * @return     True if the character is hidden from the role, false otherwise
     */
    bool hidden(ROLE role) const;
#pragma endregion
};

/**
 * @brief      Container for managing collections of character pointers.
 *
 *             This class provides a specialized container for managing groups
 *             of character pointers, typically used for player lists, party
 *             members, or other character collections with specific access
 *             patterns and management requirements.
 */
class character::container
{
private:
    std::unordered_map<uint32_t, std::shared_ptr<fb::game::character>>    _from_uid;
    std::unordered_map<std::string, std::shared_ptr<fb::game::character>> _from_name;

public:
    fb::game::server& _server;

public:
    container(fb::game::server& server);
    ~container() = default;

public:
    /**
     * @brief      Inserts a character into the container.
     *
     *             This method adds a character to the container, associating it
     *             with both its unique ID and its name. It ensures that the
     *             character is properly tracked and accessible by both methods.
     *
     * @param[in]  ch  The character to insert
     */
    void insert(std::shared_ptr<fb::game::character> ch);

    /**
     * @brief      Removes a character from the container.
     *
     * @param[in]  ch  The character to remove
     */
    void remove(std::shared_ptr<fb::game::character> ch);

    /**
     * @brief      Finds a character by their unique ID.
     *
     * @param[in]  uid  The unique ID of the character to find
     *
     * @return     A shared pointer to the character if found, nullptr otherwise
     */
    std::shared_ptr<fb::game::character> find(uint32_t uid) const;

    /**
     * @brief      Checks if a character with the given name exists in the container.
     *
     * @param[in]  name  The name of the character to check
     *
     * @return     True if the character exists, false otherwise
     */
    bool contains(const std::string& name) const;

    /**
     * @brief      Checks if a character with the given ID exists in the container.
     *
     * @param[in]  uid  The unique ID of the character to check
     *
     * @return     True if the character exists, false otherwise
     */
    bool contains(uint32_t uid) const;

    /**
     * @brief      Finds a character by their name.
     *
     * @param[in]  name  The name of the character to find
     *
     * @return     A shared pointer to the character if found, nullptr otherwise
     */
    std::shared_ptr<fb::game::character> find(const std::string& name) const;

    /**
     * @brief      Executes a function for each character in the container.
     *
     *             This method iterates through all characters in the container,
     *             executing the provided function for each character. If a character
     *             is not found, the fallback function is called with the character's
     *             ID and name.
     *
     * @param[in]  fn        The function to execute for each character
     * @param[in]  predicate   The predicate function to filter characters
     */
    async::task<void> foreach (std::function<void(std::shared_ptr<fb::game::character>&)>&&     fn,
                               std::function<bool(const std::shared_ptr<fb::game::character>&)> predicate = nullptr);

    /**
     * @brief      Executes a function for each character in the container.
     *
     * @param[in]  fn        The function to execute for each character
     * @param[in]  characters  The characters to iterate over
     */
    async::task<void> foreach (std::function<void(std::shared_ptr<fb::game::character>&)>&& fn,
                               const std::vector<std::shared_ptr<fb::game::character>>&     characters);

    /**
     * @brief      Executes a function for each character in the container asynchronously.
     *
     *             This method iterates through all characters in the container,
     *             executing the provided function for each character. If a character
     *             is not found, the fallback function is called with the character's
     *             ID and name.
     *
     * @param[in]  fn        The function to execute for each character
     * @param[in]  predict   The predicate function to filter characters
     */
    async::task<void> foreach_async(std::function<async::task<void>(std::shared_ptr<fb::game::character>&)>&& fn,
                                    std::function<bool(const std::shared_ptr<fb::game::character>&)> predict = nullptr);

    /**
     * @brief      Executes a function for each character in the container asynchronously.
     *
     * @param[in]  fn        The function to execute for each character
     * @param[in]  characters  The characters to iterate over
     */
    async::task<void> foreach_async(std::function<async::task<void>(std::shared_ptr<fb::game::character>&)>&& fn,
                                    const std::vector<std::shared_ptr<fb::game::character>>& characters);

    /**
     * @brief      Executes a function for each character in the container.
     *
     * @param[in]  names     The names of the characters to iterate over
     * @param[in]  fn        The function to execute for each character
     * @param[in]  miss      The function to call if a character is not found
     */
    async::task<void> foreach (const std::vector<std::string>& names,
                               std::function<void(std::shared_ptr<fb::game::character>&)> && fn,
                               std::function<void(const std::string& name)> miss = nullptr);

    /**
     * @brief      Executes a function for each character in the container.
     *
     * @param[in]  names     The names of the characters to iterate over
     * @param[in]  fn        The function to execute for each character
     * @param[in]  miss      The function to call if a character is not found
     */
    async::task<void> foreach_async(const std::vector<std::string>&                                           names,
                                    std::function<async::task<void>(std::shared_ptr<fb::game::character>&)>&& fn,
                                    std::function<void(const std::string& name)> miss = nullptr);

    /**
     * @brief      Invokes a function for a specific character.
     *
     * @param[in]  name  The name of the character to invoke the function for
     * @param[in]  fn    The function to invoke
     * @param[in]  miss  The function to call if a character is not found
     */
    async::task<void> invoke(const std::string&                                           name,
                             std::function<void(std::shared_ptr<fb::game::character>&)>&& fn,
                             std::function<void(const std::string& name)>                 miss = nullptr);

    /**
     * @brief      Invokes a function for a specific character asynchronously.
     *
     * @param[in]  name  The name of the character to invoke the function for
     * @param[in]  fn    The function to invoke
     * @param[in]  miss  The function to call if a character is not found
     */
    async::task<void> invoke_async(const std::string&                                                        name,
                                   std::function<async::task<void>(std::shared_ptr<fb::game::character>&)>&& fn,
                                   std::function<void(const std::string& name)> miss = nullptr);

public:
    /**
     * @brief      Accesses a character by their unique ID.
     *
     * @param[in]  uid  The unique ID of the character to access
     *
     * @return     A reference to the character
     */
    std::shared_ptr<character>& operator[] (uint32_t uid);

    /**
     * @brief      Accesses a character by their name.
     *
     * @param[in]  name  The name of the character to access
     *
     * @return     A reference to the character
     */
    std::shared_ptr<character>& operator[] (const std::string& name);
};

/**
 * @brief      Event listener interface for character-related events and notifications.
 *
 *             This interface defines all the callback methods that can be implemented
 *             to receive notifications about character state changes, interactions,
 *             and various game events. It extends multiple listener interfaces to
 *             provide comprehensive event handling for characters.
 */
struct character::listener_t : public virtual life::listener_t,
                               public virtual dialog::listener_t,
                               public virtual trade::listener_t,
                               public virtual equipment::listener_t
{
public:
    /**
     * @brief      Called when a message is sent to the character.
     *
     * @param      me       The character receiving the message
     * @param[in]  message  The message text content
     * @param[in]  type     The message type (STATE, SYSTEM, etc.)
     */
    virtual void on_message(character& me, const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE) = 0;

    /**
     * @brief      Called when a character option is changed.
     *
     * @param      me       The character whose option was changed
     * @param[in]  option   The option that was changed
     * @param[in]  enabled  Whether the option is now enabled or disabled
     */
    virtual void on_option_changed(character& me, OPTION option, bool enabled) = 0;

    /**
     * @brief      Called when character options need to be synchronized with the client.
     *
     * @param      me    The character whose options are being updated
     */
    virtual void on_update_option(character& me) = 0;

    /**
     * @brief      Called when the character's entire map view needs to be updated.
     *
     * @param      ch    The character whose map view is being updated
     * @param[in]  map   The map data to send to the character
     */
    virtual void on_update_map(character& ch, const fb::game::map& map) = 0;

    /**
     * @brief      Called when a specific region of the character's map view needs to be updated.
     *
     * @param      ch        The character whose map view is being updated
     * @param[in]  map       The map containing the updated region
     * @param[in]  position  The position of the region to update
     * @param[in]  size      The size of the region to update
     * @param[in]  crc       The CRC value of the map update
     */
    virtual void on_update_map(character&                  ch,
                               const fb::game::map&        map,
                               const fb::model::point16_t& begin,
                               const fb::model::size8_t&   size,
                               uint16_t                    crc) = 0;

    /**
     * @brief      Called when the character's background music changes.
     *
     * @param      ch      The character whose background music is changing
     * @param[in]  bgm     The new background music ID to play
     * @param[in]  volume  The volume level for the background music (0-255)
     */
    virtual void on_update_bgm(character& ch, uint16_t bgm, uint8_t volume) = 0;

    /**
     * @brief      Called when the character's buff effects need to be synchronized.
     *
     * @param      ch     The character whose buffs are being updated
     * @param[in]  buffs  The current buff effects applied to the character
     */
    virtual void on_update_buff(character& ch, const fb::game::buffs& buffs) = 0;

    /**
     * @brief      Called when the character's internal game state needs to be synchronized.
     *
     * @param      ch    The character whose internal state is being updated
     */
    virtual void on_update_internal(character& ch) = 0;

    /**
     * @brief      Called when the game time changes and needs to be sent to the character.
     *
     * @param      ch     The character receiving the time update
     * @param[in]  hours  The current game time in hours
     */
    virtual void on_update_time(character& ch, uint16_t hours) = 0;

    /**
     * @brief      Called when a character inspects another character's information.
     *
     * @param      ch      The character performing the browse/inspect action
     * @param[in]  target  The character being inspected
     */
    virtual void on_browse_character(character& ch, const character& target) = 0;

    /**
     * @brief      Called when an item tooltip should be displayed to the character.
     *
     * @param      ch        The character who should see the item tooltip
     * @param[in]  item      The item to display information for
     * @param[in]  position  The slot position of the item
     */
    virtual void on_item_tooltip(character& ch, const item& item, uint16_t position) = 0;

    /**
     * @brief      Called when the user list interface should be shown to the character.
     *
     * @param      ch    The character who should see the user list
     */
    virtual void on_show_user_list(character& ch) = 0;

    /**
     * @brief      Called when the main bulletin interface should be shown to the character.
     *
     * @param      ch    The character who should see the bulletin interface
     */
    virtual void on_show_bulletin(character& ch) = 0;

    /**
     * @brief      Called when a bulletin section with articles should be displayed to the character.
     *
     * @param      ch        The character who should see the bulletin section
     * @param[in]  section   The bulletin section to display
     * @param[in]  articles  The list of articles in the section
     * @param[in]  flag      The button enable flags for the bulletin interface
     */
    virtual void on_show_bulletin(character&                          ch,
                                  const fb::model::bulletin&          section,
                                  const std::list<bulletin::article>& articles,
                                  BULLETIN_BUTTON_ENABLE              flag) = 0;

    /**
     * @brief      Called when a specific bulletin article should be displayed to the character.
     *
     * @param      ch     The character who should see the article
     * @param[in]  value  The article to display
     * @param[in]  flag   The button enable flags for the article view
     */
    virtual void on_show_bulletin(character& ch, const bulletin::article& value, BULLETIN_BUTTON_ENABLE flag) = 0;

    /**
     * @brief      Called when the mail box with mail list should be shown to the character.
     *
     * @param      ch     The character who should see the mail box
     * @param[in]  mails  The list of mail summaries to display
     * @param[in]  flag   The button enable flags for the mail box interface
     */
    virtual void on_show_mail_box(character&                                              ch,
                                  const std::vector<fb::protocol::internal::MailSummary>& mails,
                                  MAIL_BUTTON_ENABLE                                      flag) = 0;

    /**
     * @brief      Called when a specific mail should be displayed to the character.
     *
     * @param      ch    The character who should see the mail
     * @param[in]  mail  The mail message to display
     * @param[in]  flag  The button enable flags for the mail view
     */
    virtual void on_show_mail_box(character& ch, const fb::protocol::internal::Mail& mail, MAIL_BUTTON_ENABLE flag) = 0;

    /**
     * @brief      Called when a bulletin/mail operation result message should be shown.
     *
     * @param      ch       The character who should see the message
     * @param[in]  message  The result message text to display
     * @param[in]  success  Whether the operation was successful
     * @param[in]  mail     Whether this is a mail-related message
     */
    virtual void on_show_bulletin_message(character& ch, const std::string& message, bool success, bool mail) = 0;

    /**
     * @brief      Called when the world map interface should be shown to the character.
     *
     * @param      ch     The character who should see the world map
     * @param[in]  id     The world map identifier
     * @param[in]  index  The map index to highlight or focus on
     */
    virtual void on_show_world_map(character& ch, uint32_t id, uint16_t index) = 0;

    /**
     * @brief      Called when a timer should be set for the character.
     *
     * @param      ch    The character who should receive the timer
     * @param[in]  time  The timer duration in seconds
     * @param[in]  type  The type of timer being set
     */
    virtual void on_timer(character& ch, uint32_t time, TIMER_TYPE type) = 0;

    /**
     * @brief      Called when the weather effect should change for the character.
     *
     * @param      ch       The character who should experience the weather change
     * @param[in]  weather  The new weather type to apply
     */
    virtual void on_weather(character& ch, WEATHER_TYPE weather) = 0;

    /**
     * @brief      Called when the screen brightness should change for the character.
     *
     * @param      ch     The character whose screen brightness should change
     * @param[in]  value  The new brightness level (0-255)
     */
    virtual void on_bright(character& ch, uint8_t value) = 0;

    /**
     * @brief      Called when the character's unique identifier needs to be updated.
     *
     * @param      ch    The character whose ID is being updated
     */
    virtual void on_update_id(character& ch) = 0;

    /**
     * @brief      Called when a character has been fully initialized and is ready to enter the game.
     *
     * @param      ch    The character that has been initialized
     */
    virtual void on_character_init(character& ch) = 0;

    /**
     * @brief      Called when a character's position has changed and needs to be synchronized.
     *
     * @param      ch    The character whose position has been updated
     */
    virtual void on_update_position(character& ch) = 0;

    /**
     * @brief      Called when a character gains a level.
     *
     * @param      me    The character who leveled up
     */
    virtual void on_level_up(character& me) = 0;

    /**
     * @brief      Called when a character's state needs to be synchronized with the client.
     *
     * @param      me     The character whose state is being updated
     * @param[in]  level  The update level indicating what data to synchronize
     */
    virtual void on_update(character& me, STATE_LEVEL level = STATE_LEVEL::LEVEL_MIN) = 0;

    /**
     * @brief      Called when a character is being transferred to a different map.
     *
     * @param      me        The character being transferred
     * @param      map       The destination map
     * @param[in]  position  The target position on the destination map
     *
     * @return     True if the transfer was successful, false otherwise
     */
    virtual async::task<bool> on_transfer(character& me, fb::game::map& map, const fb::model::point16_t& position) = 0;
};

} // namespace fb::game

#endif // !__CHARACTER_H__
