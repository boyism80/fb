#ifndef __LIFE_H__
#define __LIFE_H__

#include <fb/game/object.h>

namespace fb::game {

/**
 * @brief      This class describes a life.
 */
class life : public object
{
public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      { struct_description }
     */
    struct listener_t;
    /**
     * @brief      { struct_description }
     */
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
    life::listener_t& listener;
    fb::game::spells  spells;

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     * @param[in]  params   The parameters
     */
    life(fb::game::context& context, const fb::model::life& model, const initial_params& params);

    /**
     * @brief      Destroys the object.
     */
    virtual ~life();

#pragma region stat

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t base_hp() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t buff_hp() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t maxhp() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void buff_hp(uint32_t value) = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t base_mp() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t buff_mp() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t maxmp() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void buff_mp(uint32_t value) = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t base_str() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t buff_str() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t str() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void buff_str(uint8_t value) = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t base_dex() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t buff_dex() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t dex() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void buff_dex(uint8_t value) = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t base_int() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t buff_int() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t intelligence() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void buff_int(uint8_t value) = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual int8_t base_phydef() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual int8_t buff_phydef() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual int8_t phydef() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void buff_phydef(int8_t value) = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual int8_t base_magdef() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual int8_t buff_magdef() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual int8_t magdef() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void buff_magdef(int8_t value) = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t base_dam() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t buff_dam() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t dam() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void buff_dam(uint8_t value) = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t base_hit() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t buff_hit() const = 0;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t hit() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void buff_hit(uint8_t value) = 0;
#pragma endregion

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  duration  The duration
     */
    virtual void attack(DURATION duration = DURATION::ATTACK);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t hp() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void hp(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t mp() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void mp(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t exp() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     * @param      from   The from
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t heal(uint32_t value, fb::game::object* from = nullptr);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value     The value
     * @param      from      The from
     * @param[in]  critical  The critical
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t damage(uint32_t value, fb::game::object* from = nullptr, bool critical = false);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     * @param      from   The from
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t mp_up(uint32_t value, fb::game::object* from = nullptr);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     * @param      from   The from
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t mp_down(uint32_t value, fb::game::object* from = nullptr);

    /**
     * @brief      Updates the given value.
     *
     * @param[in]  value  The value
     */
    virtual void update(STATE_LEVEL value = STATE_LEVEL::LEVEL_MIN);

    /**
     * @brief      { function_description }
     *
     * @param[in]  diff      The difference
     * @param[in]  critical  The critical
     */
    void update_hp(uint32_t diff, bool critical);

    /**
     * @brief      { function_description }
     *
     * @param      from          The from
     * @param[in]  destroy_type  The destroy type
     */
    virtual void kill(fb::game::object* from = nullptr, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      { function_description }
     *
     * @return     The crowd control.
     */
    CROWD_CONTROL crowd_control() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
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
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool alive() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  spell  The spell
     *
     * @return     { description_of_the_return_value }
     */
    bool active(fb::game::spell& spell);

    /**
     * @brief      { function_description }
     *
     * @param[in]  spell  The spell
     * @param[in]  fd     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool active(fb::game::spell& spell, uint32_t fd);

    /**
     * @brief      { function_description }
     *
     * @param[in]  spell    The spell
     * @param[in]  message  The message
     *
     * @return     { description_of_the_return_value }
     */
    bool active(fb::game::spell& spell, const std::string& message);

    /**
     * @brief      { function_description }
     *
     * @param[in]  spell  The spell
     * @param      to     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool active(fb::game::spell& spell, fb::game::object& to);

    /**
     * @brief      { function_description }
     *
     * @param[in]  action    The action
     * @param[in]  duration  The duration
     * @param[in]  sound     The sound
     */
    virtual void action(ACTION action, DURATION duration, uint8_t sound = 0x00);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  size  The size
     *
     * @return     { description_of_the_return_value }
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
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t damage_rate() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void damage_rate(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t skill_damage_rate() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void skill_damage_rate(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t damage_derate() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void damage_derate(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void paralysis(bool value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool paralysis() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void invincible(bool value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool invincible() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void cover(bool value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool cover() const;

#pragma region builtin

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_message(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_hp(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_mp(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_heal(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_damage(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_mp_up(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_mp_down(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_action(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_spell(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_spells(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_cast(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_cc(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_add_cc(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_remove_cc(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_attack(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_damage_rate(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_skill_damage_rate(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_damage_derate(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_paralysis(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_invincible(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_cover(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_hp(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buff_hp(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_maxhp(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_mp(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buff_mp(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_maxmp(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_str(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buff_str(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_str(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_dex(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buff_dex(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_dex(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_int(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buff_int(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_intelligence(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_phydef(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buff_phydef(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_phydef(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_magdef(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buff_magdef(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_magdef(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_dam(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buff_dam(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_dam(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_hit(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buff_hit(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_hit(lua_State* L);
#pragma endregion
};

/**
 * @brief      { struct_description }
 */
struct life::listener_t : public virtual fb::game::object::listener_t, public virtual fb::game::spells::listener_t
{
    /**
     * @brief      Called on action.
     *
     * @param      me        { parameter_description }
     * @param[in]  action    The action
     * @param[in]  duration  The duration
     * @param[in]  sound     The sound
     */
    virtual void on_action(life& me, ACTION action, DURATION duration, uint8_t sound) = 0;

    /**
     * @brief      Called on attack.
     *
     * @param      me        { parameter_description }
     * @param[in]  duration  The duration
     */
    virtual async::task<void> on_attack(life& me, DURATION duration = DURATION::ATTACK) = 0;

    /**
     * @brief      Called on dead.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    virtual void on_dead(life& me, object* you) = 0;

    /**
     * @brief      Called on update hp.
     *
     * @param      me        { parameter_description }
     * @param[in]  diff      The difference
     * @param[in]  critical  The critical
     */
    virtual void on_update_hp(life& me, uint32_t diff, bool critical) = 0;
};

/**
 * @brief      { struct_description }
 */
struct life::initial_params : public fb::game::object::initial_params
{
public:
    uint32_t hp  = 0;
    uint32_t mp  = 0;
    uint32_t exp = 0;
};

} // namespace fb::game

#endif // !__LIFE_H__
