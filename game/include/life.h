#ifndef __LIFE_H__
#define __LIFE_H__

#include <object.h>

namespace fb { namespace game {

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
    struct listener;

    /**
     * @brief      { struct_description }
     */
    struct config : public fb::game::object::config
    {
    public:
        uint32_t hp  = 0;
        uint32_t mp  = 0;
        uint32_t exp = 0;
    };

protected:
    uint32_t  _hp = 0, _mp = 0;
    CONDITION _condition = CONDITION::NONE;

public:
    fb::game::spells spells;

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     * @param[in]  config   The configuration
     */
    life(fb::game::context& context, const fb::model::life& model, const fb::game::life::config& config);
    /**
     * @brief      Destroys the object.
     */
    virtual ~life();

protected:
    /**
     * @brief      Calculates the damage.
     *
     * @param[in]  value  The value
     * @param[in]  life   The life
     *
     * @return     The damage.
     */
    uint32_t calculate_damage(uint32_t value, const fb::game::life& life) const;

public:
    /**
     * @brief      { function_description }
     */
    virtual void attack();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t hp() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void hp(uint32_t value);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t mp() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void mp(uint32_t value);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t base_hp() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t base_mp() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t experience() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t defensive_physical() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t defensive_magical() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     * @param      from   The from
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t hp_up(uint32_t value, fb::game::object* from = nullptr);
    /**
     * @brief      { function_description }
     *
     * @param[in]  value     The value
     * @param      from      The from
     * @param[in]  critical  The critical
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t hp_down(uint32_t value, fb::game::object* from = nullptr, bool critical = false);
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
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    CONDITION condition() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    CONDITION condition_add(CONDITION value);
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    CONDITION condition_remove(CONDITION value);
    /**
     * @brief      Determines if condition contains.
     *
     * @param[in]  value  The value
     *
     * @return     True if condition contains, False otherwise.
     */
    bool condition_contains(CONDITION value) const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool alive() const;
    /**
     * @brief      { function_description }
     */
    void kill();
    /**
     * @brief      { function_description }
     *
     * @param[in]  spell  The spell
     *
     * @return     { description_of_the_return_value }
     */
    bool active(const fb::model::spell& spell);
    /**
     * @brief      { function_description }
     *
     * @param[in]  spell  The spell
     * @param[in]  fd     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool active(const fb::model::spell& spell, uint32_t fd);
    /**
     * @brief      { function_description }
     *
     * @param[in]  spell    The spell
     * @param[in]  message  The message
     *
     * @return     { description_of_the_return_value }
     */
    bool active(const fb::model::spell& spell, const std::string& message);
    /**
     * @brief      { function_description }
     *
     * @param[in]  spell  The spell
     * @param      to     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool active(const fb::model::spell& spell, fb::game::object& to);

protected:
    /**
     * @brief      Called on update.
     */
    virtual void on_update();
    /**
     * @brief      Called on calculate damage.
     *
     * @param[in]  critical  The critical
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t on_calculate_damage(bool critical) const = 0;
    /**
     * @brief      Called on calculate critical.
     *
     * @param      you   You
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool on_calculate_critical(fb::game::life& you) const;
    /**
     * @brief      Called on calculate miss.
     *
     * @param      you   You
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool on_calculate_miss(fb::game::life& you) const;
    /**
     * @brief      Called on attack.
     *
     * @param      you   You
     */
    virtual void on_attack(fb::game::object* you);
    /**
     * @brief      Called on hit.
     *
     * @param      you       You
     * @param[in]  damage    The damage
     * @param[in]  critical  The critical
     */
    virtual void on_hit(fb::game::life& you, uint32_t damage, bool critical);
    /**
     * @brief      Called when damaged.
     *
     * @param      from      The from
     * @param[in]  damage    The damage
     * @param[in]  critical  The critical
     */
    virtual void on_damaged(fb::game::object* from, uint32_t damage, bool critical);
    /**
     * @brief      Called on die.
     *
     * @param      from  The from
     */
    virtual void on_die(fb::game::object* from);

public:
    /**
     * @brief      Called on exponent.
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t on_exp() const;
    /**
     * @brief      Called on kill.
     *
     * @param      you   You
     */
    virtual void on_kill(fb::game::life& you);

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_hp(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_mp(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_hp(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_mp(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_hp_inc(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_hp_dec(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_mp_inc(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_mp_dec(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_action(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_spell(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_damage(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_cast(lua_State* lua);
};

/**
 * @brief      { struct_description }
 */
struct life::listener : public virtual fb::game::object::listener, public virtual fb::game::spells::listener
{
    /**
     * @brief      Called on attack.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    virtual void on_attack(life& me, object* you) = 0;
    /**
     * @brief      Called on hit.
     *
     * @param      me        { parameter_description }
     * @param      you       You
     * @param[in]  damage    The damage
     * @param[in]  critical  The critical
     */
    virtual void on_hit(life& me, life& you, uint32_t damage, bool critical) = 0;
    /**
     * @brief      Called on kill.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    virtual void on_kill(life& me, life& you) = 0;
    /**
     * @brief      Called when damaged.
     *
     * @param      me        { parameter_description }
     * @param      you       You
     * @param[in]  damage    The damage
     * @param[in]  critical  The critical
     */
    virtual void on_damaged(life& me, object* you, uint32_t damage, bool critical) = 0;
    /**
     * @brief      Called on die.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    virtual void on_die(life& me, object* you) = 0;
    /**
     * @brief      Called on heal hp.
     *
     * @param      me     { parameter_description }
     * @param[in]  value  The value
     * @param      from   The from
     */
    virtual void on_heal_hp(life& me, uint32_t value, fb::game::object* from) = 0;
    /**
     * @brief      Called on heal mp.
     *
     * @param      me     { parameter_description }
     * @param[in]  value  The value
     * @param      from   The from
     */
    virtual void on_heal_mp(life& me, uint32_t value, fb::game::object* from) = 0;
    /**
     * @brief      Called on hp.
     *
     * @param      me       { parameter_description }
     * @param[in]  before   The before
     * @param[in]  current  The current
     */
    virtual void on_hp(life& me, uint32_t before, uint32_t current) = 0;
    /**
     * @brief      Called on mp.
     *
     * @param      me       { parameter_description }
     * @param[in]  before   The before
     * @param[in]  current  The current
     */
    virtual void on_mp(life& me, uint32_t before, uint32_t current) = 0;
};

}} // namespace fb::game

#endif // !__LIFE_H__
