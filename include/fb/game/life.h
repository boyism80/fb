#ifndef __LIFE_H__
#define __LIFE_H__

#include <fb/game/object.h>

namespace fb { namespace game {

class life : public object
{
public:
    LUA_PROTOTYPE

public:
    interface listener;

public:
    class model;

struct config : public fb::game::object::config
{
public:
    uint32_t                    hp  = 0;
    uint32_t                    mp  = 0;
    uint32_t                    exp = 0;
};

protected:
    uint32_t                    _hp = 0, _mp = 0;
    fb::game::CONDITION_TYPE    _condition = fb::game::CONDITION_TYPE::NONE;

public:
    fb::game::spells            spells;

protected:
    life(fb::game::context& context, const model* model, const fb::game::life::config& config);
    virtual ~life();

protected:
    uint32_t                    calculate_damage(uint32_t value, const fb::game::life& life) const;

public:
    virtual void                attack();
    virtual uint32_t            hp() const;
    virtual void                hp(uint32_t value);

    virtual uint32_t            mp() const;
    virtual void                mp(uint32_t value);

    virtual uint32_t            base_hp() const;
    virtual uint32_t            base_mp() const;

    virtual uint32_t            experience() const;

    virtual uint32_t            defensive_physical() const;
    virtual uint32_t            defensive_magical() const;

    virtual uint32_t            hp_up(uint32_t value, fb::game::object* from = nullptr);
    virtual uint32_t            hp_down(uint32_t value, fb::game::object* from = nullptr, bool critical = false);
    
    virtual uint32_t            mp_up(uint32_t value, fb::game::object* from = nullptr);
    virtual uint32_t            mp_down(uint32_t value, fb::game::object* from = nullptr);

    fb::game::CONDITION_TYPE    condition() const;
    fb::game::CONDITION_TYPE    condition_add(fb::game::CONDITION_TYPE value);
    fb::game::CONDITION_TYPE    condition_remove(fb::game::CONDITION_TYPE value);
    bool                        condition_contains(fb::game::CONDITION_TYPE value) const;

    virtual bool                alive() const;
    void                        kill();

    bool                        active(fb::model::spell& spell);
    bool                        active(fb::model::spell& spell, uint32_t fd);
    bool                        active(fb::model::spell& spell, const std::string& message);
    bool                        active(fb::model::spell& spell, fb::game::object& to);

protected:
    virtual void                on_update() { }
    virtual uint32_t            on_calculate_damage(bool critical) const = 0;
    virtual bool                on_calculate_critical(fb::game::life& you) const;
    virtual bool                on_calculate_miss(fb::game::life& you) const;
    virtual void                on_attack(fb::game::object* you);
    virtual void                on_hit(fb::game::life& you, uint32_t damage, bool critical);
    virtual void                on_damaged(fb::game::object* from, uint32_t damage, bool critical);
    virtual void                on_die(fb::game::object* from);

public:
    virtual uint32_t            on_exp() const { return 0; }
    virtual void                on_kill(fb::game::life& you);


public:
    static int                  builtin_hp(lua_State* lua);
    static int                  builtin_mp(lua_State* lua);
    static int                  builtin_base_hp(lua_State* lua);
    static int                  builtin_base_mp(lua_State* lua);
    static int                  builtin_hp_inc(lua_State* lua);
    static int                  builtin_hp_dec(lua_State* lua);
    static int                  builtin_mp_inc(lua_State* lua);
    static int                  builtin_mp_dec(lua_State* lua);
    static int                  builtin_action(lua_State* lua);
    static int                  builtin_spell(lua_State* lua);
    static int                  builtin_damage(lua_State* lua);
    static int                  builtin_cast(lua_State* lua);
};

interface life::listener : public virtual fb::game::object::listener,
    public virtual fb::game::spell::listener
{
    virtual void                on_attack(life& me, object* you) = 0;
    virtual void                on_hit(life& me, life& you, uint32_t damage, bool critical) = 0;
    virtual void                on_kill(life& me, life& you) = 0;
    virtual void                on_damaged(life& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void                on_die(life& me, object* you) = 0;

    virtual void                on_heal_hp(life& me, uint32_t value, fb::game::object* from) = 0;
    virtual void                on_heal_mp(life& me, uint32_t value, fb::game::object* from) = 0;
    virtual void                on_hp(life& me, uint32_t before, uint32_t current) = 0;
    virtual void                on_mp(life& me, uint32_t before, uint32_t current) = 0;
};

} }

#endif // !__LIFE_H__
