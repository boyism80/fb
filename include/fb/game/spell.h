#ifndef __SPELL_H__
#define __SPELL_H__

#include <chrono>
#include <fb/game/inventory.h>
#include <fb/model/model.h>
#include <fb/lua.h>

namespace fb::game {

class context;

/**
 * @brief      Forward declaration of the object class.
 */
class object;
/**
 * @brief      Forward declaration of the life class.
 */
class life;

/**
 * @brief      Represents a spell instance owned by a life entity.
 *
 *             This class encapsulates a spell that can be cast by a life entity (character or mob).
 *             It manages spell cooldowns, casting delays, and provides integration with the Lua
 *             scripting system for dynamic spell behavior. Each spell instance is linked to a
 *             spell model that defines its properties and effects.
 *
 *             Features:
 *             - Cooldown and delay management for spell casting
 *             - Integration with the game context and owner entity
 *             - Lua scripting support for dynamic spell logic
 *             - Model-based configuration system
 */
class spell : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    struct builtin;

private:
    fb::model::datetime _next;

public:
    const fb::game::context& context;
    const fb::game::life&    owner;
    const fb::model::spell&  model;

public:
    /**
     * @brief      Constructs a new spell instance with specified parameters.
     *
     *             Creates a spell instance linked to the specified context, owner, and model.
     *             Optionally sets an initial delay before the spell can be cast, which is
     *             useful for implementing cooldowns or casting restrictions.
     *
     * @param[in]  context  The game context managing this spell.
     * @param[in]  owner    The life entity that owns this spell.
     * @param[in]  model    The spell model defining properties and effects.
     * @param[in]  delay    Initial delay in seconds before the spell can be cast.
     */
    spell(const fb::game::context& context,
          const fb::game::life&    owner,
          const fb::model::spell&  model,
          uint16_t                 delay = 0);
    /**
     * @brief      Destroys the spell instance.
     */
    ~spell();

public:
    /**
     * @brief      Sets the spell casting delay in seconds.
     *
     *             Updates the spell's cooldown by setting a delay from the current time.
     *             This prevents the spell from being cast until the delay period expires,
     *             implementing cooldown mechanics for balanced gameplay.
     *
     * @param[in]  value  The delay in seconds before the spell can be cast again.
     */
    void delay(uint16_t value);

    /**
     * @brief      Gets the remaining delay before the spell can be cast.
     *
     *             Calculates and returns the remaining cooldown time in seconds.
     *             Returns 0 if the spell is ready to be cast immediately.
     *
     * @return     The remaining delay in seconds, or 0 if ready to cast.
     */
    uint16_t delay() const;

    /**
     * @brief      Gets the next available casting time for this spell.
     *
     *             Returns the absolute datetime when this spell will next be available
     *             for casting, based on its current cooldown state.
     *
     * @return     The datetime when the spell will next be available.
     */
    const fb::model::datetime& next() const;
};

/**
 * @brief      Container for managing a character's spell collection.
 *
 *             This class extends the inventory template to provide specialized
 *             functionality for managing spells that a character has learned.
 *             It handles spell storage, retrieval, and provides event notifications
 *             when spells are added, removed, or modified.
 */
class spells : public fb::game::inventory<fb::game::spell>
{
public:
    using super = fb::game::inventory<fb::game::spell>;

    struct listener_t;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      owner  The life entity that owns this spell collection
     */
    spells(life& owner);
    /**
     * @brief      Destroys the object.
     */
    ~spells();

    /**
     * @brief      Searches for a spell by name in the collection.
     *
     * @param[in]  name  The name of the spell to find
     *
     * @return     Pointer to the found spell, or nullptr if not found
     */
    fb::game::spell* find(const std::string& name) const;

    /**
     * @brief      Searches for a spell by model in the collection.
     *
     * @param[in]  model  The spell model to search for
     *
     * @return     Pointer to the found spell, or nullptr if not found
     */
    fb::game::spell* find(const fb::model::spell& model) const;

public:
    /**
     * @brief      Adds a spell to the collection at the first available slot.
     *
     * @param[in]  element  The spell to add to the collection
     *
     * @return     The slot index where the spell was added, or invalid index if failed
     */
    uint8_t add(fb::game::spell& element) override;

    /**
     * @brief      Adds a spell to the collection at a specific slot index.
     *
     * @param[in]  element  The spell to add to the collection
     * @param[in]  index    The slot index where to place the spell
     *
     * @return     The slot index where the spell was added, or invalid index if failed
     */
    uint8_t add(fb::game::spell& element, uint8_t index) override;

    /**
     * @brief      Adds a spell from model data to a specific slot with delay.
     *
     * @param[in]  model  The spell model to create the spell from
     *
     * @return     The slot index where the spell was added, or invalid index if failed
     */
    uint8_t add(const fb::model::spell& model, uint8_t slot, uint16_t delay);

    /**
     * @brief      Adds a spell from model data to the first available slot.
     *
     * @param[in]  model  The spell model to create the spell from
     *
     * @return     The slot index where the spell was added, or invalid index if failed
     */
    uint8_t add(const fb::model::spell& model);

    /**
     * @brief      Removes a spell from the collection at the specified slot.
     *
     * @param[in]  index  The slot index of the spell to remove
     *
     * @return     True if the spell was successfully removed, false otherwise
     */
    bool remove(uint8_t index) override;

    /**
     * @brief      Swaps two spells between different slots in the collection.
     *
     * @param[in]  src   The source slot index
     * @param[in]  dst   The destination slot index
     *
     * @return     True if the swap was successful, false otherwise
     */
    bool swap(uint8_t src, uint8_t dst) override;
};

/**
 * @brief      Lua binding interface for spell functionality.
 */
struct spell::builtin
{
    /**
     * @brief      Lua binding for getting spell model data.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting spell delay.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_delay(lua_State* L);

    /**
     * @brief      Lua binding for setting spell delay with specific parameters.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_delay2(lua_State* L);
};

/**
 * @brief      Event listener interface for spell collection events.
 */
struct spells::listener_t
{
    /**
     * @brief      Called when a spell is updated.
     *
     * @param      me     The life entity whose spell was updated
     * @param[in]  index  The slot index of the updated spell
     */
    virtual void on_spell_update(life& me, uint8_t index) = 0;

    /**
     * @brief      Called when a spell is removed.
     *
     * @param      me     The life entity whose spell was removed
     * @param[in]  index  The slot index of the removed spell
     */
    virtual void on_spell_remove(life& me, uint8_t index) = 0;
};

/**
 * @brief      Represents a temporary effect applied to a game object.
 *
 *             This class encapsulates a buff (temporary enhancement or debuff) that
 *             can be applied to game objects. Buffs have a duration, are associated
 *             with a spell model, and can be cast by other objects. They provide
 *             Lua scripting integration for complex effect behaviors.
 */
class buff : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    struct builtin;

private:
    std::chrono::milliseconds _time;

public:
    const fb::game::context& context;
    const fb::model::spell&  model;
    const fb::game::object*  caster;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  context  The game context managing this buff
     * @param[in]  model    The spell model defining the buff's effects
     * @param[in]  caster   The object casting the buff (optional)
     * @param[in]  seconds  The duration of the buff in seconds
     */
    buff(const fb::game::context& context,
         const fb::model::spell&  model,
         const fb::game::object*  caster,
         uint32_t                 seconds);
    /**
     * @brief      Destroys the object.
     */
    ~buff();

public:
    /**
     * @brief      Gets the remaining duration of the buff.
     *
     * @return     The remaining time as milliseconds
     */
    std::chrono::milliseconds time() const;

    /**
     * @brief      Sets the buff duration using a chrono duration.
     *
     * @param[in]  value    The new duration to set
     *
     * @tparam     _Rep     The arithmetic type representing the number of ticks
     * @tparam     _Period  The std::ratio representing the tick period
     */
    template <class _Rep, class _Period>
    void time(const std::chrono::duration<_Rep, _Period>& value)
    {
        this->_time = std::chrono::duration_cast<std::chrono::milliseconds>(value);
    }

    /**
     * @brief      Increases the buff duration by the specified amount.
     *
     * @param[in]  inc   The duration to add to the current time
     */
    void time_inc(const std::chrono::steady_clock::duration& inc);

    /**
     * @brief      Decreases the buff duration by the specified amount.
     *
     * @param[in]  dec   The duration to subtract from the current time
     */
    void time_dec(const std::chrono::steady_clock::duration& dec);
};

/**
 * @brief      Container for managing active buffs on a game object.
 *
 *             This class manages all active temporary effects (buffs and debuffs)
 *             applied to a game object. It provides efficient lookup by spell ID,
 *             automatic duration management, and event notifications for buff
 *             application and removal.
 */
class buffs : private std::unordered_map<uint32_t, buff*>
{
private:
    using super = std::unordered_map<uint32_t, buff*>;

private:
    fb::game::object& _owner;

public:
    using super::begin;
    using super::cbegin;
    using super::cend;
    using super::contains;
    using super::end;
    using super::erase;
    using super::insert;
    using super::size;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      owner  The game object that owns these buffs
     */
    buffs(fb::game::object& owner);
    /**
     * @brief      Destroys the object.
     */
    ~buffs();

private:
    /**
     * @brief      Adds a buff to the collection (internal method).
     *
     * @param      buff  The buff to add
     *
     * @return     True if the buff was successfully added, false otherwise
     */
    bool push_back(buff& buff);

public:
    /**
     * @brief      Checks if a buff from the specified spell model is active.
     *
     * @param[in]  model  The spell model to check for
     *
     * @return     True if a buff from this spell is active, false otherwise
     */
    bool contains(const fb::model::spell& model) const;

    /**
     * @brief      Creates and adds a new buff from spell model data.
     *
     * @param[in]  spell    The spell model defining the buff's effects
     * @param[in]  seconds  The duration of the buff in seconds
     * @param[in]  caster   The object casting the buff (optional)
     *
     * @return     Pointer to the created buff, or nullptr if failed
     */
    buff* push_back(const fb::model::spell& spell, uint32_t seconds, const fb::game::object* caster = nullptr);

    /**
     * @brief      Removes a buff by its spell ID.
     *
     * @param[in]  id    The spell ID of the buff to remove
     *
     * @return     True if the buff was successfully removed, false otherwise
     */
    bool remove(uint32_t id);

    /**
     * @brief      Removes a buff by its spell model.
     *
     * @param[in]  spell  The spell model defining the buff's effects
     *
     * @return     True if the buff was successfully removed, false otherwise
     */
    bool remove(const fb::model::spell& spell);

public:
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  id    The spell ID of the buff to access
     *
     * @return     The result of the array indexer
     */
    buff* operator[] (uint32_t id) const;
};

/**
 * @brief      Lua binding interface for buff functionality.
 */
struct buff::builtin
{
    /**
     * @brief      Lua binding for getting buff model data.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting buff duration.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_time(lua_State* L);
};

} // namespace fb::game

#endif // !__SPELL_H__
