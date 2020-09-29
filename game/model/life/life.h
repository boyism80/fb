#ifndef __LIFE_H__
#define __LIFE_H__

#include "model/object/object.h"

namespace fb { namespace game {

class listener;

class life : public object
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion


#pragma region master class
public:
    class master : public fb::game::object::master
    {
#pragma region lua
    public:
        LUA_PROTOTYPE
#pragma endregion


#pragma region protected field
    protected:
        fb::game::defensive         _defensive;
        uint32_t                    _hp, _mp;
        uint32_t                    _experience;
#pragma endregion


#pragma region friend
    public:
        friend class fb::game::life;
#pragma endregion


#pragma region constructor / destructor
    public:
        master(const std::string& name, uint16_t look, uint8_t color, uint32_t hp, uint32_t mp);
        master(const master& master, uint32_t hp, uint32_t mp);
        master(const master& master);
        virtual ~master();
#pragma endregion


#pragma region public method
    public:
        uint32_t                    hp() const;
        void                        hp(uint32_t value);

        uint32_t                    mp() const;
        void                        mp(uint32_t value);

        uint32_t                    experience() const;
        void                        experience(uint32_t value);

        uint32_t                    defensive_physical() const;
        void                        defensive_physical(uint8_t value);

        uint32_t                    defensive_magical() const;
        void                        defensive_magical(uint8_t value);

        object*                     make(listener* listener) const;
#pragma endregion


#pragma region built-in method
    public:
        static int                  builtin_hp(lua_State* lua);
        static int                  builtin_mp(lua_State* lua);
#pragma endregion
    };

#pragma endregion


#pragma region private field
private:
    listener*                       _listener;
#pragma endregion


#pragma region protected field
protected:
    uint32_t                    _hp, _mp;
    fb::game::condition         _condition;
#pragma endregion


#pragma region public field
public:
    fb::game::spells            spells;
#pragma endregion


#pragma region constructor / destructor
protected:
    life(const master* master, listener* listener);
    life(master* master, listener* listener, uint32_t id, uint32_t hp = 0, uint32_t mp = 0, uint32_t exp = 0);
    life(const fb::game::object& object, listener* listener, uint32_t hp, uint32_t mp, uint32_t exp);
    virtual ~life();
#pragma endregion


#pragma region protected method
protected:
    uint32_t                    random_damage(uint32_t value, const fb::game::life& life) const;
#pragma endregion


#pragma region public method
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

    fb::game::condition         condition() const;
    fb::game::condition         condition_add(fb::game::condition value);
    fb::game::condition         condition_remove(fb::game::condition value);
    bool                        condition_contains(fb::game::condition value) const;

    virtual bool                alive() const;
    void                        kill();
#pragma endregion


#pragma region built-in method
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
#pragma endregion
};

} }

#endif // !__LIFE_H__
