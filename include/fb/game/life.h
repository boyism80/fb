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
    struct listener;

    struct initial_params : public fb::game::object::initial_params
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
    life(fb::game::context& context, const fb::model::life& model, const initial_params& params);
    virtual ~life();

public:
    virtual void     attack();
    uint32_t hp() const;
    void     hp(uint32_t value);
    uint32_t mp() const;
    void     mp(uint32_t value);
    virtual uint32_t base_hp() const;
    virtual uint32_t base_mp() const;
    virtual uint32_t exp() const;
    virtual uint32_t defensive_physical() const;
    virtual uint32_t defensive_magical() const;
    virtual uint32_t heal(uint32_t value, fb::game::object* from = nullptr);
    virtual uint32_t damage(uint32_t value, fb::game::object* from = nullptr, bool critical = false);
    virtual uint32_t mp_up(uint32_t value, fb::game::object* from = nullptr);
    virtual uint32_t mp_down(uint32_t value, fb::game::object* from = nullptr);
    virtual void update(STATE_LEVEL value = STATE_LEVEL::LEVEL_MIN);
    void update_hp(uint32_t diff, bool critical);

    virtual void kill(fb::game::object* from = nullptr, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    CONDITION    condition() const;
    CONDITION    condition_add(CONDITION value);
    CONDITION    condition_remove(CONDITION value);
    bool         condition_contains(CONDITION value) const;
    virtual bool alive() const;
    bool         active(const fb::model::spell& spell);
    bool         active(const fb::model::spell& spell, uint32_t fd);
    bool         active(const fb::model::spell& spell, const std::string& message);
    bool         active(const fb::model::spell& spell, fb::game::object& to);
    virtual void action(ACTION action, DURATION duration, uint8_t sound = 0x00);

public:
    virtual uint32_t auto_attack_damage(MOB_SIZE size) const = 0;

    virtual bool calculate_critical(life& you) const;

    virtual uint32_t calculate_damage(uint32_t damage, const life& you, bool critical) const;
    virtual bool     calculate_miss(life& you) const;

public:
    static int builtin_hp(lua_State* lua);
    static int builtin_mp(lua_State* lua);
    static int builtin_base_hp(lua_State* lua);
    static int builtin_base_mp(lua_State* lua);
    static int builtin_hp_inc(lua_State* lua);
    static int builtin_hp_dec(lua_State* lua);
    static int builtin_mp_inc(lua_State* lua);
    static int builtin_mp_dec(lua_State* lua);
    static int builtin_action(lua_State* lua);
    static int builtin_spell(lua_State* lua);
    static int builtin_damage(lua_State* lua);
    static int builtin_cast(lua_State* lua);
};

struct life::listener : public virtual fb::game::object::listener, public virtual fb::game::spells::listener
{
    virtual void on_action(life& me, ACTION action, DURATION duration, uint8_t sound)                              = 0;
    virtual void on_attack(life& me)                                                                               = 0;
    virtual void on_dead(life& me, object* you)                                                                    = 0;
    virtual void on_update_hp(life& me, uint32_t diff, bool critical) = 0;
};

} // namespace fb::game

#endif // !__LIFE_H__
