#ifndef __ITEM_H__
#define __ITEM_H__

/**
 * @file    item.h
 * @brief   Item and equipment system for the FB 2D MMORPG game server
 * @author  FB Development Team
 *
 * @details This file implements the comprehensive item and equipment system that
 *          handles all game items including weapons, armor, consumables, and
 *          miscellaneous items. The system provides functionality for item
 *          management, equipment mechanics, inventory operations, and trading.
 *
 *          Key features:
 *          - Base item class with count-based stacking for stackable items
 *          - Equipment system with weapons, armor, shields, helmets, rings, and accessories
 *          - Durability system for equipment degradation and repair mechanics
 *          - Comprehensive inventory management with storage and retrieval
 *          - Trade system integration with item transfer capabilities
 *          - Drop and loot mechanics with timing and ownership tracking
 *          - Death-related item handling for player death scenarios
 *          - Custom naming system for weapons and special items
 *          - Protocol serialization for network communication
 *          - Lua scripting integration for dynamic item behavior
 *          - Thread-safe operations for multi-threaded server environment
 *
 * @note    This system handles all aspects of item existence from creation to
 *          destruction, including inventory management and player interactions.
 */

#include <iomanip>
#include <fb/game/object.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/model/datetime.h>

#ifdef small
#undef small
#endif

namespace fb::game {

/**
 * @brief      Forward declaration of the character class.
 */
class character;
/**
 * @brief      Forward declaration of the items container class.
 */
class items;

/**
 * @brief      Represents an item object in the game world.
 *
 *             This class extends the object class to provide functionality specific to items
 *             that can be picked up, carried, traded, and used by characters. It manages item
 *             properties such as count, durability, trade status, and container relationships.
 *             Items can exist in character inventories, on the ground, or in storage containers.
 *
 *             Key features:
 *             - Count-based stacking for stackable items
 *             - Durability system for equipment degradation
 *             - Trade system integration with trade counts
 *             - Container management for inventory systems
 *             - Drop and loot mechanics with timing
 *             - Death-related item handling
 *             - Lua scripting integration for dynamic behavior
 *             - Protocol serialization for network communication
 */
class item : public object
{
public:
    using model_type = fb::model::item;

public:
    using object::map;

public:
    friend class items;

public:
    struct builtin;
    struct listener_t;
    struct conditions;

public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      Initial parameters for item creation.
     *
     *             This structure extends the base object parameters to include
     *             item-specific initialization data such as stack count.
     */
    struct initial_params : fb::game::object::initial_params
    {
    public:
        uint16_t count = 1;
    };

public:
    static const conditions DEFAULT_CONDITION;

public:
    using nullable_time = std::optional<fb::model::datetime>;

protected:
    uint16_t                _count        = 0;
    uint16_t                _trade_count  = 0;
    items*                  _container    = nullptr;
    std::optional<uint32_t> _death_cid    = std::nullopt;
    nullable_time           _dropped_time = std::nullopt;

public:
    listener_t& listener;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context         The game context managing this item
     * @param[in]  model           The item model data containing properties and stats
     * @param[in]  params          The initial parameters for the item
     */
    item(fb::game::context&     context,
         const fb::model::item& model,
         const initial_params&  params = initial_params{.count = 1});
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source item to copy from
     */
    item(const item& right);
    /**
     * @brief      Destroys the object.
     */
    virtual ~item();

public:
    std::shared_ptr<fb::game::character> owner() const;

public:
    /**
     * @brief      Places the item on a map at the specified position.
     *
     * @param      map           The target map to place the item on
     * @param[in]  position      The position coordinates on the map
     * @param[in]  destroy_type  The destruction type when placing the item
     * @param[in]  notify        Whether to notify other objects about the move
     *
     * @return     Async task that returns true if placement was successful
     */
    virtual async::task<bool> map(std::shared_ptr<fb::game::map> map,
                                  const fb::model::point16_t&    position,
                                  DESTROY_TYPE                   destroy_type = DESTROY_TYPE::DEFAULT,
                                  bool                           notify       = true) override;

    /**
     * @brief      Gets the tooltip message for the item.
     *
     * @return     String containing the item's tooltip information
     */
    virtual std::string tip_message() const;

public:
    /**
     * @brief      Gets the item's current durability.
     *
     * @return     Optional durability value, or nullopt if item has no durability
     */
    virtual std::optional<uint32_t> durability() const;
    /**
     * @brief      Sets the item's durability.
     *
     * @param[in]  value  The new durability value
     */
    virtual void durability(uint32_t value);

public:
    /**
     * @brief      Gets the item name for inventory display.
     *
     * @return     String containing the item name as shown in inventory
     */
    virtual std::string inven_name() const;
    /**
     * @brief      Gets the item name for trade display.
     *
     * @return     String containing the item name as shown in trade windows
     */
    virtual std::string trade_name() const;
    /**
     * @brief      Fills the item stack with the specified count.
     *
     * @param[in]  count  The number of items to add to the stack
     *
     * @return     The actual number of items added to the stack
     */
    uint16_t fill(uint16_t count);
    /**
     * @brief      Gets the remaining space in the item stack.
     *
     * @return     The number of items that can still be added to the stack
     */
    uint16_t free_space() const;
    /**
     * @brief      Gets the current item count in the stack.
     *
     * @return     The number of items in the current stack
     */
    uint16_t count() const;
    /**
     * @brief      Sets the item count in the stack.
     *
     * @param[in]  value  The new item count
     */
    void count(uint16_t value);

    /**
     * @brief      Gets the item count for trading purposes.
     *
     * @return     The number of items available for trade
     */
    uint16_t trade_count() const;

    /**
     * @brief      Sets the item count for trading purposes.
     *
     * @param[in]  value  The new trade count
     */
    void trade_count(uint16_t value);

    /**
     * @brief      Checks if the item stack is empty.
     *
     * @return     True if the item count is zero, false otherwise
     */
    virtual bool empty() const;
    /**
     * @brief      Gets the time when the item was dropped.
     *
     * @return     Reference to the nullable time when item was dropped
     */
    const nullable_time& dropped_time() const;

public:
    /**
     * @brief      Sets the character ID that caused this item to drop on death.
     *
     * @param[in]  cid   The character ID that caused the death drop
     */
    void death_cid(std::optional<uint32_t> cid);
    /**
     * @brief      Gets the character ID that caused this item to drop on death.
     *
     * @return     Optional character ID, or nullopt if not a death drop
     */
    std::optional<uint32_t> death_cid() const;

public:
    /**
     * @brief      Checks if the item is currently active/usable.
     *
     * @return     True if the item can be used, false otherwise
     */
    virtual bool active();
    /**
     * @brief      Splits the item stack into a separate item.
     *
     * @param[in]  count  The number of items to split off
     *
     * @return     Pointer to the new split item, or nullptr if split failed
     */
    virtual std::shared_ptr<fb::game::item> split(uint16_t count = 1);
    /**
     * @brief      Merges another item into this item stack.
     *
     * @param      item  The item to merge into this stack
     */
    virtual void merge(std::shared_ptr<fb::game::item> item);

    /**
     * @brief      Gets the thread this item belongs to.
     *
     * @return     Pointer to the thread managing this item
     */
    fb::thread* thread() const override final;

    /**
     * @brief      Asserts that the current thread is the correct thread for this item.
     */
    void assert_thread() const override;

public:
    void container(fb::game::items* container);

public:
    /**
     * @brief      Returns a protocol representation of the object.
     *
     * @return     Protocol representation of the object.
     */
    virtual fb::protocol::internal::Item to_protocol(EQUIPMENT_PARTS parts = EQUIPMENT_PARTS::UNKNOWN) const;
};

struct item::builtin
{
    /**
     * @brief      Lua builtin function to get item model information.
     *
     * @param      L   The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_model(lua_State* L);
    /**
     * @brief      Lua builtin function to get/set item count.
     *
     * @param      L   The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_count(lua_State* L);
    /**
     * @brief      Lua builtin function to get/set item durability.
     *
     * @param      L   The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_durability(lua_State* L);
    /**
     * @brief      Lua builtin function to rename an item.
     *
     * @param      L   The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_rename(lua_State* L);
};

/**
 * @brief      Event listener interface for item-specific events.
 *
 *             This interface extends the object listener to provide event handling
 *             specifically for item operations such as removal, updates, swapping,
 *             activation, and throwing.
 */
struct item::listener_t : public virtual object::listener_t
{
    /**
     * @brief      Called when an item is removed from a character's inventory.
     *
     * @param      me     The character whose item is being removed
     * @param[in]  index  The inventory slot index of the item being removed
     * @param[in]  attr   The deletion type indicating how the item was removed
     */
    virtual void on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE) = 0;
    /**
     * @brief      Called when an item in a character's inventory needs to be synchronized with the client.
     *
     * @param      me     The character whose item was updated
     * @param[in]  index  The inventory slot index of the item that was updated
     */
    virtual void on_item_update(character& me, uint8_t index) = 0;
    /**
     * @brief      Called when two items are swapped between inventory slots.
     *
     * @param      me    The character performing the item swap
     * @param[in]  src   The source inventory slot index
     * @param[in]  dst   The destination inventory slot index
     */
    virtual void on_item_swap(character& me, uint8_t src, uint8_t dst) = 0;
    /**
     * @brief      Called when a character activates or uses an item.
     *
     * @param      me    The character activating the item
     * @param      item  The item being activated or used
     */
    virtual void on_item_active(character& me, item& item) = 0;
    /**
     * @brief      Called when a character throws an item to a specific location.
     *
     * @param      me    The character throwing the item
     * @param      item  The item being thrown
     * @param[in]  to    The target position coordinates where the item is thrown
     */
    virtual void on_item_throws(character& me, item& item, const fb::model::point16_t& to) = 0;
};

/**
 * @brief      Represents cash/money items in the game.
 *
 *             This class handles monetary items that can be picked up, traded,
 *             and used for transactions. Cash items have special handling for
 *             stacking and value calculations.
 */
class cash : public item
{
public:
    const uint32_t value = 0;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this cash item
     * @param[in]  chunk    The initial cash value
     */
    cash(fb::game::context& context, uint32_t chunk);
    /**
     * @brief      Destroys the object.
     */
    ~cash();

private:
    /**
     * @brief      Finds the cash model that matches the specified value.
     *
     * @param      context  The game context
     * @param[in]  value    The cash value to match
     *
     * @return     Reference to the matching cash model
     */
    static const fb::model::cash& match_model(fb::game::context& context, uint32_t value);

public:
    /**
     * @brief      Replaces this cash item with a new value.
     *
     * @param[in]  value  The new cash value
     *
     * @return     Shared pointer to the new cash item, or nullptr if failed
     */
    std::shared_ptr<fb::game::cash> replace(uint32_t value);
    /**
     * @brief      Reduces the cash amount by the specified value.
     *
     * @param[in]  value  The amount to reduce
     *
     * @return     The actual amount reduced
     */
    uint32_t reduce(uint32_t value);

    /**
     * @brief      Checks if the cash item is empty (zero value).
     *
     * @return     True if the cash value is zero, false otherwise
     */
    bool empty() const;
    /**
     * @brief      Gets the cash item name for inventory display.
     *
     * @return     String containing the cash item name with value
     */
    std::string inven_name() const override;
};

/**
 * @brief      Represents consumable items that can be used by players.
 *
 *             This class handles items that can be consumed for various effects,
 *             such as potions, food, scrolls, and other single-use items. Consumables
 *             typically have usage restrictions and provide temporary benefits.
 */
class consume : public item
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this consumable item
     * @param[in]  model    The consumable item model data containing usage properties
     * @param[in]  count    The initial quantity of consumable items in the stack
     */
    consume(fb::game::context& context, const fb::model::consume& model, uint16_t count = 1);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source consumable item to copy from
     */
    consume(const consume& right);
    /**
     * @brief      Destroys the object.
     */
    ~consume();

public:
    /**
     * @brief      Checks if the consumable item can be used.
     *
     * @return     True if the item can be consumed, false otherwise
     */
    bool active() override;
};

/**
 * @brief      Represents container items that can hold other items.
 *
 *             This class handles pack/bag items that provide additional inventory
 *             space for players. Packs can contain other items and have their own
 *             capacity limitations and access restrictions.
 */
class pack : public item
{
private:
    uint32_t _durability = 0;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this pack item
     * @param[in]  model    The pack model data containing capacity and properties
     */
    pack(fb::game::context& context, const fb::model::pack& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source pack item to copy from
     */
    pack(const pack& right);
    /**
     * @brief      Destroys the object.
     */
    ~pack();

public:
    /**
     * @brief      Gets the pack's current durability.
     *
     * @return     Optional durability value, or nullopt if pack has no durability
     */
    std::optional<uint32_t> durability() const;
    /**
     * @brief      Sets the pack's durability.
     *
     * @param[in]  value  The new durability value
     */
    void durability(uint32_t value);
    /**
     * @brief      Gets the pack name for inventory display.
     *
     * @return     String containing the pack name as shown in inventory
     */
    std::string inven_name() const override;

public:
    /**
     * @brief      Checks if the pack can be used/opened.
     *
     * @return     True if the pack can be opened, false otherwise
     */
    bool active() final;
};

/**
 * @brief      Base class for all equippable items.
 *
 *             This class serves as the foundation for all items that can be equipped
 *             by characters, such as weapons, armor, accessories, and other gear.
 *             Equipment items provide stat bonuses, have durability, and can be
 *             repaired when damaged.
 */
class equipment : public item
{
public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      Event listener interface for equipment-specific events.
     *
     *             This interface extends the item listener to provide event handling
     *             specifically for equipment items, including equipping, unequipping,
     *             and durability changes.
     */
    struct listener_t;

protected:
    uint32_t _durability = 0;

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this equipment item
     * @param[in]  model    The equipment model data containing stats and properties
     */
    equipment(fb::game::context& context, const fb::model::equipment& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source equipment item to copy from
     */
    equipment(const fb::game::equipment& right);

public:
    /**
     * @brief      Destroys the object.
     */
    virtual ~equipment();

public:
    /**
     * @brief      Checks if the equipment can be used/equipped.
     *
     * @return     True if the equipment can be equipped, false otherwise
     */
    bool active() override;

public:
    /**
     * @brief      Gets the equipment's current durability.
     *
     * @return     Optional durability value, or nullopt if equipment has no durability
     */
    std::optional<uint32_t> durability() const;
    /**
     * @brief      Sets the equipment's durability.
     *
     * @param[in]  value  The new durability value
     */
    void durability(uint32_t value);

    /**
     * @brief      Reduces equipment durability by the specified amount.
     *
     * @param[in]  value  The amount to reduce durability by
     *
     * @return     True if equipment broke (durability reached 0), false otherwise
     */
    bool durability_down(uint32_t value);

protected:
    /**
     * @brief      Gets the middle section of the equipment tooltip message.
     *
     * @return     String containing the middle portion of tooltip information
     */
    virtual std::string mid_message() const;

public:
    /**
     * @brief      Gets the tooltip message for the equipment.
     *
     * @return     String containing the equipment's tooltip information
     */
    virtual std::string tip_message() const;
    /**
     * @brief      Gets the equipment name for trade display.
     *
     * @return     String containing the equipment name as shown in trade windows
     */
    virtual std::string trade_name() const override;

public:
    /**
     * @brief      Gets the database column name for the specified equipment part.
     *
     * @param[in]  parts  The equipment part type
     *
     * @return     String containing the database column name
     */
    static const std::string column(EQUIPMENT_PARTS parts);
};

/**
 * @brief      Event listener interface for equipment-specific events.
 *
 *             This interface extends the item listener to provide event handling
 *             specifically for equipment items, including equipping, unequipping,
 *             and durability changes.
 */
struct equipment::listener_t : public virtual fb::game::item::listener_t
{
    /**
     * @brief      Called when a character equips an equipment item.
     *
     * @param      me     The character equipping the equipment
     * @param      item   The equipment item being equipped
     * @param[in]  parts  The equipment slot where the item is being equipped
     */
    virtual void on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts) = 0;
    /**
     * @brief      Called when a character unequips an equipment item.
     *
     * @param      me         The character unequipping the equipment
     * @param[in]  parts      The equipment slot being unequipped
     * @param      equipment  The equipment item being removed from the slot
     */
    virtual void on_equipment_off(character& me, EQUIPMENT_PARTS parts, fb::game::equipment& equipment) = 0;

    /**
     * @brief      Called when equipment durability decreases due to damage or usage.
     *
     * @param      me         The character whose equipment lost durability
     * @param      equipment  The equipment item that lost durability
     * @param[in]  before     The durability value before the decrease
     * @param[in]  after      The durability value after the decrease
     */
    virtual void on_durability_down(character& me, fb::game::equipment& equipment, uint32_t before, uint32_t after) = 0;
};

/**
 * @brief      Represents weapon equipment that can be wielded for combat.
 *
 *             This class handles all types of melee weapons that characters can
 *             equip for combat. Weapons provide damage bonuses, have attack
 *             animations, and may have special combat properties.
 */
class weapon : public equipment
{
public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      Enumeration of weapon types for classification and restrictions.
     *
     *             This enumeration defines the different categories of weapons
     *             available in the game, used for equipment restrictions,
     *             skill requirements, and combat mechanics.
     */
    enum class types : uint8_t
    {
        NORMAL,
        SPEAR,
        BOW,
        FAN,
        UNKNOWN
    };

private:
    std::optional<std::string> _custom_name;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this weapon
     * @param[in]  model    The weapon model data containing damage and properties
     */
    weapon(fb::game::context& context, const fb::model::weapon& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source weapon to copy from
     */
    weapon(const fb::game::weapon& right);
    /**
     * @brief      Destroys the object.
     */
    ~weapon();

protected:
    /**
     * @brief      Gets the middle section of the weapon tooltip message.
     *
     * @return     String containing the weapon-specific tooltip information
     */
    std::string mid_message() const override final;

public:
    /**
     * @brief      Gets the weapon's display name.
     *
     * @return     Reference to the weapon's name (custom or default)
     */
    const std::string& name() const override final;
    /**
     * @brief      Gets the weapon name for inventory display.
     *
     * @return     String containing the weapon name as shown in inventory
     */
    std::string inven_name() const override;
    /**
     * @brief      Gets the weapon name for trade display.
     *
     * @return     String containing the weapon name as shown in trade windows
     */
    std::string trade_name() const override;
    /**
     * @brief      Gets the weapon's custom name if set.
     *
     * @return     Optional custom name, or nullopt if no custom name is set
     */
    const std::optional<std::string>& custom_name() const;
    /**
     * @brief      Sets a custom name for the weapon.
     *
     * @param[in]  name  The custom name to assign to the weapon
     */
    void custom_name(const std::string& name);
    /**
     * @brief      Resets the weapon's custom name to default.
     */
    void reset_custom_name();
    /**
     * @brief      Returns a protocol representation of the object.
     *
     * @return     Protocol representation of the object.
     */
    fb::protocol::internal::Item to_protocol(EQUIPMENT_PARTS parts = EQUIPMENT_PARTS::UNKNOWN) const override;
};

/**
 * @brief      Represents body armor equipment for defense.
 *
 *             This class handles armor items that provide defensive bonuses
 *             and protection against physical and magical attacks. Armor
 *             affects the character's appearance and defensive capabilities.
 */
class armor : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this armor
     * @param[in]  model    The armor model data containing defense and properties
     */
    armor(fb::game::context& context, const fb::model::armor& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source armor to copy from
     */
    armor(const fb::game::armor& right);
    /**
     * @brief      Destroys the object.
     */
    ~armor();
};

/**
 * @brief      Represents head protection equipment.
 *
 *             This class handles helmet and hat items that provide head
 *             protection and may offer various stat bonuses. Helmets
 *             affect the character's head appearance and defensive stats.
 */
class helmet : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this helmet
     * @param[in]  model    The helmet model data containing defense and properties
     */
    helmet(fb::game::context& context, const fb::model::helmet& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source helmet to copy from
     */
    helmet(const helmet& right);
    /**
     * @brief      Destroys the object.
     */
    ~helmet();
};

/**
 * @brief      Represents shield equipment for blocking attacks.
 *
 *             This class handles shield items that provide defensive capabilities
 *             and can be used to block incoming attacks. Shields offer protection
 *             bonuses and may have special defensive properties.
 */
class shield : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this shield
     * @param[in]  model    The shield model data containing defense and properties
     */
    shield(fb::game::context& context, const fb::model::shield& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source shield to copy from
     */
    shield(const shield& right);
    /**
     * @brief      Destroys the object.
     */
    ~shield();
};

/**
 * @brief      Represents ring accessories that provide stat bonuses.
 *
 *             This class handles ring items that can be worn as accessories
 *             to provide various stat enhancements and special effects.
 *             Characters can typically wear multiple rings simultaneously.
 */
class ring : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this ring
     * @param[in]  model    The ring model data containing enchantments and properties
     */
    ring(fb::game::context& context, const fb::model::ring& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source ring to copy from
     */
    ring(const ring& right);
    /**
     * @brief      Destroys the object.
     */
    ~ring();
};

/**
 * @brief      Represents auxiliary equipment and accessories.
 *
 *             This class handles miscellaneous equipment items that don't
 *             fit into other categories, such as special accessories,
 *             utility items, or unique equipment pieces.
 */
class auxiliary : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this auxiliary item
     * @param[in]  model    The auxiliary model data containing effects and properties
     */
    auxiliary(fb::game::context& context, const fb::model::auxiliary& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source auxiliary item to copy from
     */
    auxiliary(const auxiliary& right);
    /**
     * @brief      Destroys the object.
     */
    ~auxiliary();
};

/**
 * @brief      Container for managing a character's item inventory.
 *
 *             This class extends the inventory template to provide specialized
 *             functionality for managing items that a character carries.
 *             It handles item storage, stacking, trading, and provides event
 *             notifications for inventory changes.
 */
class items : public fb::game::inventory<fb::game::item>
{
private:
    using super = fb::game::inventory<fb::game::item>;

private:
    std::weak_ptr<fb::game::character>           _owner;
    std::shared_ptr<fb::game::weapon>            _weapon         = nullptr;
    std::shared_ptr<fb::game::armor>             _armor          = nullptr;
    std::shared_ptr<fb::game::helmet>            _helmet         = nullptr;
    std::shared_ptr<fb::game::shield>            _shield         = nullptr;
    std::shared_ptr<fb::game::ring>              _rings[2]       = {nullptr, nullptr};
    std::shared_ptr<fb::game::auxiliary>         _auxiliaries[2] = {nullptr, nullptr};
    std::vector<std::shared_ptr<fb::game::item>> _stored;
    uint32_t                                     _deposited = 0;

public:
    /**
     * @brief      Constructs a new instance.
     */
    items();
    /**
     * @brief      Destroys the object.
     */
    ~items();

public:
    /**
     * @brief      Sets the owner of this item container.
     *
     * @param[in]  owner  The character who owns this container
     */
    void owner(std::shared_ptr<fb::game::character> owner);

    /**
     * @brief      Gets the owner of this item container.
     *
     * @return     The character who owns this container
     */
    std::shared_ptr<fb::game::character> owner() const;

    /**
     * @brief      Adds the specified item to the inventory.
     *
     * @param      item  The item to add
     *
     * @return     The inventory slot index where the item was added, or 0xFF if failed
     */
    uint8_t add(std::shared_ptr<fb::game::item> item) override;
    /**
     * @brief      Adds multiple items to the inventory.
     *
     * @param[in]  items             The vector of items to add
     * @param[in]  stop_if_remained  Whether to stop adding if any item cannot be fully added
     *
     * @return     Vector of inventory slot indices where items were added
     */
    std::vector<uint8_t> add(const std::vector<std::shared_ptr<fb::game::item>>& items, bool stop_if_remained = false);
    /**
     * @brief      Adds an item to a specific inventory slot.
     *
     * @param      item   The item to add
     * @param[in]  index  The inventory slot index to add the item to
     *
     * @return     The inventory slot index where the item was added, or 0xFF if failed
     */
    uint8_t add(std::shared_ptr<fb::game::item> item, uint8_t index) override;
    /**
     * @brief      Stores an item in the storage/warehouse using smart pointer.
     *
     * @param      item  The shared pointer to the item to store
     *
     * @return     True if the item was successfully stored, false otherwise
     */
    bool store(std::shared_ptr<fb::game::item> item);
    /**
     * @brief      Stores items from inventory slot to storage.
     *
     * @param[in]  index  The inventory slot index
     * @param[in]  count  The number of items to store
     *
     * @return     True if the items were successfully stored, false otherwise
     */
    bool store(uint8_t index, uint16_t count);
    /**
     * @brief      Stores items by name to storage.
     *
     * @param[in]  name   The item name to store
     * @param[in]  count  The number of items to store
     *
     * @return     True if the items were successfully stored, false otherwise
     */
    bool store(const std::string& name, uint16_t count);
    /**
     * @brief      Gets a stored item by model type.
     *
     * @param[in]  item  The item model to search for
     *
     * @return     Pointer to the stored item, or nullptr if not found
     */
    std::shared_ptr<fb::game::item> stored(const fb::model::item& item) const;
    /**
     * @brief      Gets all stored items in the storage.
     *
     * @return     Reference to the vector of all stored items
     */
    const std::vector<std::shared_ptr<fb::game::item>>& stored() const;
    /**
     * @brief      Retrieves items from storage by index.
     *
     * @param[in]  index  The storage slot index
     * @param[in]  count  The number of items to retrieve
     *
     * @return     Pointer to the retrieved item, or nullptr if failed
     */
    std::shared_ptr<fb::game::item> retrieve(uint8_t index, uint16_t count);
    /**
     * @brief      Retrieves items from storage by name.
     *
     * @param[in]  name   The item name to retrieve
     * @param[in]  count  The number of items to retrieve
     *
     * @return     Pointer to the retrieved item, or nullptr if failed
     */
    std::shared_ptr<fb::game::item> retrieve(const std::string& name, uint16_t count);
    /**
     * @brief      Retrieves items from storage by model.
     *
     * @param[in]  item   The item model to retrieve
     * @param[in]  count  The number of items to retrieve
     *
     * @return     Pointer to the retrieved item, or nullptr if failed
     */
    std::shared_ptr<fb::game::item> retrieve(const fb::model::item& item, uint16_t count);
    /**
     * @brief      Gets the amount of money deposited in storage.
     *
     * @return     The current deposited money amount
     */
    uint32_t deposited() const;
    /**
     * @brief      Sets the amount of money deposited in storage.
     *
     * @param[in]  value  The new deposited money amount
     */
    void deposited(uint32_t value);
    /**
     * @brief      Deposits money into storage.
     *
     * @param[in]  value  The amount of money to deposit
     *
     * @return     The actual amount deposited
     */
    uint32_t deposit(uint32_t value);
    /**
     * @brief      Withdraws money from storage.
     *
     * @param[in]  value  The amount of money to withdraw
     *
     * @return     The actual amount withdrawn
     */
    uint32_t withdraw(uint32_t value);
    /**
     * @brief      Activates an item at the specified inventory slot.
     *
     * @param[in]  index  The inventory slot index
     *
     * @return     Pointer to the activated item, or nullptr if failed
     */
    std::shared_ptr<fb::game::item> active(uint8_t index);
    /**
     * @brief      Unequips equipment from the specified slot.
     *
     * @param[in]  parts  The equipment slot to unequip
     *
     * @return     The inventory slot index where the equipment was moved
     */
    uint8_t inactive(EQUIPMENT_PARTS parts);
    /**
     * @brief      Removes equipment from the specified slot.
     *
     * @param[in]  parts  The equipment slot to remove from
     *
     * @return     Pointer to the removed equipment, or nullptr if slot was empty
     */
    std::shared_ptr<fb::game::equipment> equipment_off(EQUIPMENT_PARTS parts);

    /**
     * @brief      Finds the inventory slot index of an item by model.
     *
     * @param[in]  item  The item model to search for
     *
     * @return     The inventory slot index, or 0xFF if not found
     */
    uint8_t index(const fb::model::item& item) const;
    /**
     * @brief      Finds the inventory slot index of a specific item instance.
     *
     * @param[in]  item  The item instance to search for
     *
     * @return     The inventory slot index, or 0xFF if not found
     */
    uint8_t index(const std::shared_ptr<fb::game::item>& item) const;
    /**
     * @brief      Finds all inventory slot indices containing the specified item model.
     *
     * @param[in]  item  The item model to search for
     *
     * @return     Vector of inventory slot indices containing the item
     */
    std::vector<uint8_t> index_all(const std::shared_ptr<fb::game::item>& item) const;

    /**
     * @brief      Updates the inventory slot to reflect changes.
     *
     * @param[in]  index  The inventory slot index to update
     *
     * @return     True if the update was successful, false otherwise
     */
    bool update(uint8_t index) const;
    /**
     * @brief      Equips an item to the specified equipment slot.
     *
     * @param[in]  parts  The equipment slot to equip to
     * @param      item   The equipment item to wear
     *
     * @return     Pointer to the previously equipped item, or nullptr if slot was empty
     */
    std::shared_ptr<fb::game::equipment> wear(EQUIPMENT_PARTS parts, std::shared_ptr<fb::game::equipment> item);
    /**
     * @brief      Gets the currently equipped weapon.
     *
     * @return     Pointer to the equipped weapon, or nullptr if no weapon equipped
     */
    std::shared_ptr<fb::game::weapon> weapon() const;
    /**
     * @brief      Equips a weapon and returns the previously equipped weapon.
     *
     * @param      weapon  The weapon to equip
     *
     * @return     Pointer to the previously equipped weapon, or nullptr if none
     */
    std::shared_ptr<fb::game::weapon> weapon(std::shared_ptr<fb::game::weapon> weapon);
    /**
     * @brief      Gets the currently equipped armor.
     *
     * @return     Pointer to the equipped armor, or nullptr if no armor equipped
     */
    std::shared_ptr<fb::game::armor> armor() const;
    /**
     * @brief      Equips armor and returns the previously equipped armor.
     *
     * @param      armor  The armor to equip
     *
     * @return     Pointer to the previously equipped armor, or nullptr if none
     */
    std::shared_ptr<fb::game::armor> armor(std::shared_ptr<fb::game::armor> armor);
    /**
     * @brief      Gets the currently equipped shield.
     *
     * @return     Pointer to the equipped shield, or nullptr if no shield equipped
     */
    std::shared_ptr<fb::game::shield> shield() const;
    /**
     * @brief      Equips a shield and returns the previously equipped shield.
     *
     * @param      shield  The shield to equip
     *
     * @return     Pointer to the previously equipped shield, or nullptr if none
     */
    std::shared_ptr<fb::game::shield> shield(std::shared_ptr<fb::game::shield> shield);
    /**
     * @brief      Gets the currently equipped helmet.
     *
     * @return     Pointer to the equipped helmet, or nullptr if no helmet equipped
     */
    std::shared_ptr<fb::game::helmet> helmet() const;
    /**
     * @brief      Equips a helmet and returns the previously equipped helmet.
     *
     * @param      helmet  The helmet to equip
     *
     * @return     Pointer to the previously equipped helmet, or nullptr if none
     */
    std::shared_ptr<fb::game::helmet> helmet(std::shared_ptr<fb::game::helmet> helmet);
    /**
     * @brief      Gets the ring equipped at the specified position.
     *
     * @param[in]  position  The ring slot position (left or right)
     *
     * @return     Pointer to the equipped ring, or nullptr if slot is empty
     */
    std::shared_ptr<fb::game::ring> ring(EQUIPMENT_POSITION position) const;
    /**
     * @brief      Equips a ring to the first available ring slot.
     *
     * @param      ring  The ring to equip
     *
     * @return     Pointer to the previously equipped ring, or nullptr if none
     */
    std::shared_ptr<fb::game::ring> ring(std::shared_ptr<fb::game::ring> ring);
    /**
     * @brief      Equips a ring to the specified position.
     *
     * @param      ring      The ring to equip
     * @param[in]  position  The ring slot position to equip to
     *
     * @return     Pointer to the previously equipped ring, or nullptr if none
     */
    std::shared_ptr<fb::game::ring> ring(std::shared_ptr<fb::game::ring> ring, EQUIPMENT_POSITION position);
    /**
     * @brief      Gets the auxiliary item equipped at the specified position.
     *
     * @param[in]  position  The auxiliary slot position
     *
     * @return     Pointer to the equipped auxiliary item, or nullptr if slot is empty
     */
    std::shared_ptr<fb::game::auxiliary> auxiliary(EQUIPMENT_POSITION position) const;
    /**
     * @brief      Equips an auxiliary item to the first available auxiliary slot.
     *
     * @param      auxiliary  The auxiliary item to equip
     *
     * @return     Pointer to the previously equipped auxiliary, or nullptr if none
     */
    std::shared_ptr<fb::game::auxiliary> auxiliary(std::shared_ptr<fb::game::auxiliary> auxiliary);
    /**
     * @brief      Equips an auxiliary item to the specified position.
     *
     * @param      auxiliary  The auxiliary item to equip
     * @param[in]  position   The auxiliary slot position to equip to
     *
     * @return     Pointer to the previously equipped auxiliary, or nullptr if none
     */
    std::shared_ptr<fb::game::auxiliary> auxiliary(std::shared_ptr<fb::game::auxiliary> auxiliary,
                                                   EQUIPMENT_POSITION                   position);
    /**
     * @brief      Searches for an item by name in the inventory.
     *
     * @param[in]  name  The item name to search for
     *
     * @return     Pointer to the first matching item, or nullptr if not found
     */
    std::shared_ptr<fb::game::item> find(const std::string& name) const;
    /**
     * @brief      Searches for an item by model in the inventory.
     *
     * @param[in]  model  The item model to search for
     *
     * @return     Pointer to the first matching item, or nullptr if not found
     */
    std::shared_ptr<fb::game::item> find(const fb::model::item& model) const;
    /**
     * @brief      Drops items from inventory to the ground.
     *
     * @param[in]  index        The inventory slot index
     * @param[in]  count        The number of items to drop
     * @param[in]  action       Whether to perform drop action animation
     * @param[in]  delete_type  The type of deletion/drop
     *
     * @return     Pointer to the dropped item, or nullptr if drop failed
     */
    std::shared_ptr<fb::game::item>
    drop(uint8_t index, uint8_t count, bool action = true, ITEM_DELETE_TYPE delete_type = ITEM_DELETE_TYPE::DROP);
    /**
     * @brief      Picks up items from the ground automatically.
     *
     * @param[in]  boost  Whether to use loot boost/enhancement
     */
    void loot(bool boost);
    /**
     * @brief      Throws an item from inventory.
     *
     * @param[in]  index  The inventory slot index to throw
     * @param[in]  all    Whether to throw all items
     *
     * @return     True if the item was successfully thrown, false otherwise
     */
    bool throws(uint8_t index, bool all);
    /**
     * @brief      Gets all currently equipped items.
     *
     * @return     Map of equipment parts to equipped items
     */
    std::map<EQUIPMENT_PARTS, std::shared_ptr<equipment>> equipments() const;
    /**
     * @brief      Swaps items between two inventory slots.
     *
     * @param[in]  src   The source inventory slot index
     * @param[in]  dst   The destination inventory slot index
     *
     * @return     True if the swap was successful, false otherwise
     */
    bool swap(uint8_t src, uint8_t dst) override;

    /**
     * @brief      Removes items from the specified inventory slot.
     *
     * @param[in]  index   The inventory slot index
     * @param[in]  count   The number of items to remove
     * @param[in]  attr    The deletion type attribute
     * @param[in]  detach  Whether to detach the item from inventory
     *
     * @return     Pointer to the removed item, or nullptr if removal failed
     */
    std::shared_ptr<fb::game::item>
    remove(uint8_t index, uint16_t count = 1, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE, bool detach = true);
    /**
     * @brief      Removes a specific item from inventory.
     *
     * @param      item    The item instance to remove
     * @param[in]  count   The number of items to remove
     * @param[in]  attr    The deletion type attribute
     * @param[in]  detach  Whether to detach the item from inventory
     *
     * @return     Pointer to the removed item, or nullptr if removal failed
     */
    std::shared_ptr<fb::game::item> remove(std::shared_ptr<fb::game::item> item,
                                           uint16_t                        count  = 1,
                                           ITEM_DELETE_TYPE                attr   = ITEM_DELETE_TYPE::NONE,
                                           bool                            detach = true);
};

} // namespace fb::game

#endif // !__ITEM_H__
