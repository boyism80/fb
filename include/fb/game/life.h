#ifndef __LIFE_H__
#define __LIFE_H__

#include <fb/game/object.h>

namespace fb::game {

/**
 * @brief      Base class for all living entities in the game world.
 *
 *             This abstract class extends the object class to provide functionality specific
 *             to living entities such as characters and mobs. It manages health points (HP),
 *             magic points (MP), combat statistics, spell casting, buff/debuff effects, and
 *             various status conditions that affect living entities.
 *
 *             Key features:
 *             - HP/MP management with base and buffed values
 *             - Complete combat statistics system (STR, DEX, INT, damage, defense)
 *             - Spell system integration with casting and cooldowns
 *             - Buff/debuff system with crowd control effects
 *             - Combat mechanics (damage calculation, critical hits, miss chance)
 *             - Status effects (paralysis, invincibility, cover)
 *             - Lua scripting integration for dynamic behavior
 *             - Event-driven architecture with listener pattern
 */
class life : public object
{
public:
    LUA_PROTOTYPE

public:
    struct builtin;
    struct listener_t;
    struct initial_params;

protected:
    uint32_t      _hp = 0, _mp = 0;
    uint32_t      _damage_rate = 1000, _skill_damage_rate = 1000;
    uint32_t      _damage_derate = 1000;
    CROWD_CONTROL _crowd_control = CROWD_CONTROL::NONE;
    bool          _paralysis     = false;
    bool          _invincible    = false;
    bool          _cover         = false;

public:
    listener_t&      listener;
    fb::game::spells spells;

protected:
    /**
     * @brief      Constructs a new life entity with specified parameters.
     *
     *             Initializes a life entity with the provided context, model configuration,
     *             and initial parameters. Sets up HP/MP values, combat statistics, and
     *             spell system based on the model data.
     *
     * @param      context  The game context managing this life entity.
     * @param[in]  model    The life model containing base statistics and configuration.
     * @param[in]  params   Initial parameters for entity creation.
     */
    life(fb::game::context& context, const fb::model::life& model, const initial_params& params);

    /**
     * @brief      Destroys the life entity and cleans up resources.
     *
     *             Ensures proper cleanup of spells, buffs, and other resources
     *             associated with this life entity.
     */
    virtual ~life();

#pragma region stat

public:
    /**
     * @brief      Gets the base HP value without buffs.
     *
     * @return     The base HP value
     */
    virtual uint32_t base_hp() const = 0;

    /**
     * @brief      Gets the HP bonus from buffs and equipment.
     *
     * @return     The buffed HP value
     */
    virtual uint32_t buff_hp() const = 0;

    /**
     * @brief      Gets the maximum HP (base + buff).
     *
     * @return     The maximum HP value
     */
    virtual uint32_t maxhp() const;

    /**
     * @brief      Sets the HP buff value.
     *
     * @param[in]  value  The new HP buff value
     */
    virtual void buff_hp(uint32_t value) = 0;

    /**
     * @brief      Gets the base MP value without buffs.
     *
     * @return     The base MP value
     */
    virtual uint32_t base_mp() const = 0;

    /**
     * @brief      Gets the MP bonus from buffs and equipment.
     *
     * @return     The buffed MP value
     */
    virtual uint32_t buff_mp() const = 0;

    /**
     * @brief      Gets the maximum MP (base + buff).
     *
     * @return     The maximum MP value
     */
    virtual uint32_t maxmp() const;

    /**
     * @brief      Sets the MP buff value.
     *
     * @param[in]  value  The new MP buff value
     */
    virtual void buff_mp(uint32_t value) = 0;

    /**
     * @brief      Gets the base strength value without buffs.
     *
     * @return     The base strength value
     */
    virtual uint8_t base_str() const = 0;

    /**
     * @brief      Gets the strength bonus from buffs and equipment.
     *
     * @return     The buffed strength value
     */
    virtual uint8_t buff_str() const = 0;

    /**
     * @brief      Gets the total strength (base + buff).
     *
     * @return     The total strength value
     */
    virtual uint8_t str() const;

    /**
     * @brief      Sets the strength buff value.
     *
     * @param[in]  value  The new strength buff value
     */
    virtual void buff_str(uint8_t value) = 0;

    /**
     * @brief      Gets the base dexterity value without buffs.
     *
     * @return     The base dexterity value
     */
    virtual uint8_t base_dex() const = 0;

    /**
     * @brief      Gets the dexterity bonus from buffs and equipment.
     *
     * @return     The buffed dexterity value
     */
    virtual uint8_t buff_dex() const = 0;

    /**
     * @brief      Gets the total dexterity (base + buff).
     *
     * @return     The total dexterity value
     */
    virtual uint8_t dex() const;

    /**
     * @brief      Sets the dexterity buff value.
     *
     * @param[in]  value  The new dexterity buff value
     */
    virtual void buff_dex(uint8_t value) = 0;

    /**
     * @brief      Gets the base intelligence value without buffs.
     *
     * @return     The base intelligence value
     */
    virtual uint8_t base_int() const = 0;

    /**
     * @brief      Gets the intelligence bonus from buffs and equipment.
     *
     * @return     The buffed intelligence value
     */
    virtual uint8_t buff_int() const = 0;

    /**
     * @brief      Gets the total intelligence (base + buff).
     *
     * @return     The total intelligence value
     */
    virtual uint8_t intelligence() const;

    /**
     * @brief      Sets the intelligence buff value.
     *
     * @param[in]  value  The new intelligence buff value
     */
    virtual void buff_int(uint8_t value) = 0;

    /**
     * @brief      Gets the base physical defense value without buffs.
     *
     * @return     The base physical defense value
     */
    virtual int8_t base_phydef() const = 0;

    /**
     * @brief      Gets the physical defense bonus from buffs and equipment.
     *
     * @return     The buffed physical defense value
     */
    virtual int8_t buff_phydef() const = 0;

    /**
     * @brief      Gets the total physical defense (base + buff).
     *
     * @return     The total physical defense value
     */
    virtual int8_t phydef() const;

    /**
     * @brief      Sets the physical defense buff value.
     *
     * @param[in]  value  The new physical defense buff value
     */
    virtual void buff_phydef(int8_t value) = 0;

    /**
     * @brief      Gets the base magical defense value without buffs.
     *
     * @return     The base magical defense value
     */
    virtual int8_t base_magdef() const = 0;

    /**
     * @brief      Gets the magical defense bonus from buffs and equipment.
     *
     * @return     The buffed magical defense value
     */
    virtual int8_t buff_magdef() const = 0;

    /**
     * @brief      Gets the total magical defense (base + buff).
     *
     * @return     The total magical defense value
     */
    virtual int8_t magdef() const;

    /**
     * @brief      Sets the magical defense buff value.
     *
     * @param[in]  value  The new magical defense buff value
     */
    virtual void buff_magdef(int8_t value) = 0;

    /**
     * @brief      Gets the base damage value without buffs.
     *
     * @return     The base damage value
     */
    virtual uint8_t base_dam() const = 0;

    /**
     * @brief      Gets the damage bonus from buffs and equipment.
     *
     * @return     The buffed damage value
     */
    virtual uint8_t buff_dam() const = 0;

    /**
     * @brief      Gets the total damage (base + buff).
     *
     * @return     The total damage value
     */
    virtual uint8_t dam() const;

    /**
     * @brief      Sets the damage buff value.
     *
     * @param[in]  value  The new damage buff value
     */
    virtual void buff_dam(uint8_t value) = 0;

    /**
     * @brief      Gets the base hit accuracy value without buffs.
     *
     * @return     The base hit accuracy value
     */
    virtual uint8_t base_hit() const = 0;

    /**
     * @brief      Gets the hit accuracy bonus from buffs and equipment.
     *
     * @return     The buffed hit accuracy value
     */
    virtual uint8_t buff_hit() const = 0;

    /**
     * @brief      Gets the total hit accuracy (base + buff).
     *
     * @return     The total hit accuracy value
     */
    virtual uint8_t hit() const;

    /**
     * @brief      Sets the hit accuracy buff value.
     *
     * @param[in]  value  The new hit accuracy buff value
     */
    virtual void buff_hit(uint8_t value) = 0;
#pragma endregion

public:
    /**
     * @brief      Performs an attack action with specified duration.
     *
     * @param[in]  duration  The attack duration type
     */
    virtual void attack(DURATION duration = DURATION::ATTACK);

    /**
     * @brief      Gets the current HP value.
     *
     * @return     The current health points
     */
    uint32_t hp() const;

    /**
     * @brief      Sets the current HP value.
     *
     * @param[in]  value  The new HP value to set
     */
    void hp(uint32_t value);

    /**
     * @brief      Gets the current MP value.
     *
     * @return     The current mana points
     */
    uint32_t mp() const;

    /**
     * @brief      Sets the current MP value.
     *
     * @param[in]  value  The new MP value to set
     */
    void mp(uint32_t value);

    /**
     * @brief      Gets the current experience points.
     *
     * @return     The current experience points
     */
    virtual uint32_t exp() const;

    /**
     * @brief      Heals the life entity by the specified amount.
     *
     * @param[in]  value  The amount of HP to heal
     * @param      from   The source object causing the heal (optional)
     *
     * @return     The actual amount of HP healed
     */
    virtual uint32_t heal(uint32_t value, fb::game::object* from = nullptr);

    /**
     * @brief      Applies damage to the life entity.
     *
     * @param[in]  value     The base damage amount
     * @param      from      The source object causing damage (optional)
     * @param[in]  critical  Whether this is a critical hit
     *
     * @return     The actual damage dealt
     */
    virtual uint32_t damage(uint32_t value, fb::game::object* from = nullptr, bool critical = false);

    /**
     * @brief      Increases MP by the specified amount.
     *
     * @param[in]  value  The amount of MP to restore
     * @param      from   The source object causing the MP increase (optional)
     *
     * @return     The actual amount of MP restored
     */
    virtual uint32_t mp_up(uint32_t value, fb::game::object* from = nullptr);

    /**
     * @brief      Decreases MP by the specified amount.
     *
     * @param[in]  value  The amount of MP to consume
     * @param      from   The source object causing the MP decrease (optional)
     *
     * @return     The actual amount of MP consumed
     */
    virtual uint32_t mp_down(uint32_t value, fb::game::object* from = nullptr);

    /**
     * @brief      Updates the given value.
     *
     * @param[in]  value  The value
     */
    virtual void update(STATE_LEVEL value = STATE_LEVEL::LEVEL_MIN);

    /**
     * @brief      Updates HP display with damage/heal effects.
     *
     * @param[in]  diff      The HP difference amount (positive for heal, negative for damage)
     * @param[in]  critical  Whether this was a critical hit/heal
     */
    void update_hp(uint32_t diff, bool critical);

    /**
     * @brief      Kills the life entity and handles death effects.
     *
     * @param      from          The object that caused the death (optional)
     * @param[in]  destroy_type  How the entity should be destroyed
     */
    virtual void kill(fb::game::object* from = nullptr, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      Gets the current crowd control effects.
     *
     * @return     The current crowd control status
     */
    CROWD_CONTROL crowd_control() const;

    /**
     * @brief      Sets the crowd control effects.
     *
     * @param[in]  value  The new crowd control status to set
     */
    void crowd_control(CROWD_CONTROL value);

    /**
     * @brief      Adds a cc.
     *
     * @param[in]  value  The value
     *
     * @return     The crowd control.
     */
    CROWD_CONTROL add_cc(CROWD_CONTROL value);

    /**
     * @brief      Removes a cc.
     *
     * @param[in]  value  The value
     *
     * @return     The crowd control.
     */
    CROWD_CONTROL remove_cc(CROWD_CONTROL value);

    /**
     * @brief      Determines if condition contains.
     *
     * @param[in]  value  The value
     *
     * @return     True if condition contains, False otherwise.
     */
    bool condition_contains(CROWD_CONTROL value) const;

    /**
     * @brief      Checks if the life entity is alive.
     *
     * @return     True if HP is greater than 0, false otherwise
     */
    virtual bool alive() const;

    /**
     * @brief      Activates a spell without target.
     *
     * @param[in]  spell  The spell to activate
     *
     * @return     True if spell was successfully activated, false otherwise
     */
    bool active(fb::game::spell& spell);

    /**
     * @brief      Activates a spell with a file descriptor target.
     *
     * @param[in]  spell  The spell to activate
     * @param[in]  fd     The file descriptor of the target
     *
     * @return     True if spell was successfully activated, false otherwise
     */
    bool active(fb::game::spell& spell, uint32_t fd);

    /**
     * @brief      Activates a spell with a message parameter.
     *
     * @param[in]  spell    The spell to activate
     * @param[in]  message  The message parameter for the spell
     *
     * @return     True if spell was successfully activated, false otherwise
     */
    bool active(fb::game::spell& spell, const std::string& message);

    /**
     * @brief      Activates a spell targeting another object.
     *
     * @param[in]  spell  The spell to activate
     * @param      to     The target object for the spell
     *
     * @return     True if spell was successfully activated, false otherwise
     */
    bool active(fb::game::spell& spell, fb::game::object& to);

    /**
     * @brief      Performs an action with visual and audio effects.
     *
     * @param[in]  action    The action type to perform
     * @param[in]  duration  The duration of the action
     * @param[in]  sound     The sound effect ID to play
     */
    virtual void action(ACTION action, DURATION duration, uint8_t sound = 0x00);

public:
    /**
     * @brief      Calculates auto-attack damage based on target size.
     *
     * @param[in]  size  The target mob size (affects damage calculation)
     *
     * @return     The calculated auto-attack damage value
     */
    virtual uint32_t auto_attack_damage(MOB_SIZE size) const = 0;

    /**
     * @brief      Calculates the critical.
     *
     * @param      you   You
     *
     * @return     The critical.
     */
    virtual bool calculate_critical(life& you) const;

    /**
     * @brief      Calculates the damage.
     *
     * @param[in]  damage    The damage
     * @param[in]  you       You
     * @param[in]  critical  The critical
     *
     * @return     The damage.
     */
    virtual uint32_t calculate_damage(uint32_t damage, const life& you, bool critical) const;

    /**
     * @brief      Calculates the miss.
     *
     * @param      you   You
     *
     * @return     The miss.
     */
    virtual bool calculate_miss(life& you) const;

    /**
     * @brief      Gets the damage rate multiplier.
     *
     * @return     The current damage rate multiplier (1000 = 100%)
     */
    uint32_t damage_rate() const;

    /**
     * @brief      Sets the damage rate multiplier.
     *
     * @param[in]  value  The new damage rate multiplier to set
     */
    void damage_rate(uint32_t value);

    /**
     * @brief      Gets the skill damage rate multiplier.
     *
     * @return     The current skill damage rate multiplier (1000 = 100%)
     */
    uint32_t skill_damage_rate() const;

    /**
     * @brief      Sets the skill damage rate multiplier.
     *
     * @param[in]  value  The new skill damage rate multiplier to set
     */
    void skill_damage_rate(uint32_t value);

    /**
     * @brief      Gets the damage reduction rate.
     *
     * @return     The current damage reduction rate (1000 = 100%)
     */
    uint32_t damage_derate() const;

    /**
     * @brief      Sets the damage reduction rate.
     *
     * @param[in]  value  The new damage reduction rate to set
     */
    void damage_derate(uint32_t value);

    /**
     * @brief      Sets the paralysis status effect.
     *
     * @param[in]  value  True to enable paralysis, false to disable
     */
    void paralysis(bool value);

    /**
     * @brief      Checks if the entity is paralyzed.
     *
     * @return     True if paralyzed, false otherwise
     */
    bool paralysis() const;

    /**
     * @brief      Sets the invincibility status effect.
     *
     * @param[in]  value  True to enable invincibility, false to disable
     */
    void invincible(bool value);

    /**
     * @brief      Checks if the entity is invincible.
     *
     * @return     True if invincible, false otherwise
     */
    bool invincible() const;

    /**
     * @brief      Sets the cover status effect.
     *
     * @param[in]  value  True to enable cover, false to disable
     */
    void cover(bool value);

    /**
     * @brief      Checks if the entity has cover protection.
     *
     * @return     True if under cover, false otherwise
     */
    bool cover() const;
};

/**
 * @brief      Lua binding interface for life entity functionality.
 *
 *             This structure provides static methods that expose life entity
 *             functionality to Lua scripts, allowing script-based control
 *             of health, mana, combat, spells, and status effects.
 */
struct life::builtin
{
    /**
     * @brief      Lua binding for sending messages to life entities.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_message(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting HP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_hp(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting MP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_mp(lua_State* L);

    /**
     * @brief      Lua binding for healing life entities.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_heal(lua_State* L);

    /**
     * @brief      Lua binding for dealing damage to life entities.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_damage(lua_State* L);

    /**
     * @brief      Lua binding for restoring MP to life entities.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_mp_up(lua_State* L);

    /**
     * @brief      Lua binding for consuming MP from life entities.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_mp_down(lua_State* L);

    /**
     * @brief      Lua binding for performing actions.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_action(lua_State* L);

    /**
     * @brief      Lua binding for accessing individual spells.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_spell(lua_State* L);

    /**
     * @brief      Lua binding for accessing the spell collection.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_spells(lua_State* L);

    /**
     * @brief      Lua binding for casting spells.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_cast(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting crowd control effects.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_cc(lua_State* L);

    /**
     * @brief      Lua binding for adding crowd control effects.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_add_cc(lua_State* L);

    /**
     * @brief      Lua binding for removing crowd control effects.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_remove_cc(lua_State* L);

    /**
     * @brief      Lua binding for performing attacks.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_attack(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting damage rate multipliers.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_damage_rate(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting skill damage rate multipliers.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_skill_damage_rate(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting damage reduction rates.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_damage_derate(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting paralysis status.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_paralysis(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting invincibility status.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_invincible(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting cover status.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_cover(lua_State* L);

    /**
     * @brief      Lua binding for getting base HP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_hp(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting HP buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_hp(lua_State* L);

    /**
     * @brief      Lua binding for getting maximum HP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_maxhp(lua_State* L);

    /**
     * @brief      Lua binding for getting base MP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_mp(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting MP buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_mp(lua_State* L);

    /**
     * @brief      Lua binding for getting maximum MP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_maxmp(lua_State* L);

    /**
     * @brief      Lua binding for getting base strength values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_str(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting strength buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_str(lua_State* L);

    /**
     * @brief      Lua binding for getting total strength values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_str(lua_State* L);

    /**
     * @brief      Lua binding for getting base dexterity values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_dex(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting dexterity buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_dex(lua_State* L);

    /**
     * @brief      Lua binding for getting total dexterity values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_dex(lua_State* L);

    /**
     * @brief      Lua binding for getting base intelligence values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_int(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting intelligence buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_int(lua_State* L);

    /**
     * @brief      Lua binding for getting total intelligence values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_intelligence(lua_State* L);

    /**
     * @brief      Lua binding for getting base physical defense values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_phydef(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting physical defense buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_phydef(lua_State* L);

    /**
     * @brief      Lua binding for getting total physical defense values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_phydef(lua_State* L);

    /**
     * @brief      Lua binding for getting base magical defense values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_magdef(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting magical defense buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_magdef(lua_State* L);

    /**
     * @brief      Lua binding for getting total magical defense values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_magdef(lua_State* L);

    /**
     * @brief      Lua binding for getting base damage values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_dam(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting damage buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_dam(lua_State* L);

    /**
     * @brief      Lua binding for getting total damage values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_dam(lua_State* L);

    /**
     * @brief      Lua binding for getting base hit accuracy values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_hit(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting hit accuracy buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_hit(lua_State* L);

    /**
     * @brief      Lua binding for getting total hit accuracy values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_hit(lua_State* L);
};

/**
 * @brief      Event listener interface for life entity events.
 *
 *             This interface extends both object and spells listeners to provide
 *             comprehensive event handling for living entities. It includes
 *             callbacks for combat actions, health changes, death events,
 *             and other life-specific behaviors.
 */
struct life::listener_t : public virtual fb::game::object::listener_t, public virtual fb::game::spells::listener_t
{
    /**
     * @brief      Called when a life entity performs an action.
     *
     * @param      me        The life entity performing the action
     * @param[in]  action    The action type being performed
     * @param[in]  duration  The duration of the action
     * @param[in]  sound     The sound effect ID to play
     */
    virtual void on_action(life& me, ACTION action, DURATION duration, uint8_t sound) = 0;

    /**
     * @brief      Called when a life entity performs an attack.
     *
     * @param      me        The life entity performing the attack
     * @param[in]  duration  The attack duration type
     */
    virtual async::task<void> on_attack(life& me, DURATION duration = DURATION::ATTACK) = 0;

    /**
     * @brief      Called when a life entity dies.
     *
     * @param      me    The life entity that died
     * @param      you   The object that caused the death (optional)
     */
    virtual void on_dead(life& me, object* you) = 0;

    /**
     * @brief      Called when a life entity's HP changes and needs visual update.
     *
     * @param      me        The life entity whose HP changed
     * @param[in]  diff      The HP difference amount (positive for heal, negative for damage)
     * @param[in]  critical  Whether this was a critical hit/heal
     */
    virtual void on_update_hp(life& me, uint32_t diff, bool critical) = 0;
};

/**
 * @brief      Initialization parameters for creating life entities.
 *
 *             This structure extends object initialization parameters to include
 *             life-specific attributes such as health points, mana points, and
 *             experience. These parameters are used during life entity construction
 *             to set up the initial vital statistics.
 */
struct life::initial_params : public fb::game::object::initial_params
{
public:
    uint32_t hp  = 0; ///< Initial hit points (health)
    uint32_t mp  = 0; ///< Initial mana points (magic power)
    uint32_t exp = 0; ///< Initial experience points
};

} // namespace fb::game

#endif // !__LIFE_H__
