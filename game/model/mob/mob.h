#ifndef __MOB_H__
#define __MOB_H__

#include "model/life/life.h"
#include "model/item/item.h"


namespace fb { namespace game {

class session;
class listener;

class mob : public life
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion


#pragma region structure
public:
    typedef struct _damage
    {
    public:
        uint16_t min, max;

    public:
        _damage() : _damage(0, 0) {}
        _damage(uint16_t min, uint16_t max) : min(min), max(max) {}
    } damage;

    typedef struct _drop
    {
    public:
        float percentage;
        const fb::game::item::master* item;

    public:
        _drop() : percentage(0), item(NULL) {}
        _drop(const fb::game::item::master* item, float percentage) : item(item), percentage(percentage) {}
        _drop(const _drop& right) : percentage(right.percentage), item(right.item) {}
    } drop;
#pragma endregion


#pragma region enum
public:
    enum offensive_type : uint8_t { CONTAINMENT, COUNTER, NONE, NON_MOVE, RUN_AWAY};

    enum sizes : uint8_t { SMALL = 0x00, LARGE = 0x01 };
#pragma endregion


#pragma region master class
public:
class master : public fb::game::life::master
{
public:
#pragma region lua
    LUA_PROTOTYPE
#pragma endregion


#pragma region private field
private:
    mob::damage                 _damage;
    offensive_type              _offensive_type;
    sizes                       _size;
    uint32_t                    _speed;
    std::string                 _script_attack, _script_die;
    std::vector<drop>           _items;
#pragma endregion


#pragma region friend
public:
    friend class mob;
#pragma endregion


#pragma region constructor / destructor
public:
    master(const std::string& name, uint16_t look, uint8_t color, uint32_t hp, uint32_t mp);
    master(const life::master& master);
    ~master();
#pragma endregion


#pragma region public method
public:
    object::types               type() const { return fb::game::object::types::MOB; }
    object*                     make(listener* listener) const;

public:
    uint16_t                    damage_min() const;
    void                        damage_min(uint16_t value);

    uint16_t                    damage_max() const;
    void                        damage_max(uint16_t value);

    mob::sizes                  size() const;
    void                        size(mob::sizes value);

    uint32_t                    speed() const;
    void                        speed(uint32_t value);

    const std::string&          script_attack() const;
    void                        script_attack(const std::string& value);

    const std::string&          script_die() const;
    void                        script_die(const std::string& value);

    offensive_type              offensive() const;
    void                        offensive(offensive_type value);

    void                        dropitem_add(const mob::drop& money);
    void                        dropitem_add(const fb::game::item::master* item, float percentage);
    const std::vector<drop>&    items() const;
#pragma endregion


#pragma region built-in method
public:
    static int                  builtin_speed(lua_State* lua);
#pragma endregion
};
#pragma endregion


#pragma region private field
private:
    listener*                   _listener;
    point16_t                   _spawn_point;
    size16_t                    _spawn_size;

    uint64_t                    _action_time; // ms
    uint64_t                    _dead_time; // ms
    uint32_t                    _respawn_time; // seconds

    fb::game::life*             _target;
    lua::thread*                _attack_thread;
#pragma endregion
    

#pragma region constructor / destructor
public:
    mob(const mob::master* master, listener* listener, bool alive = false);
    mob(const mob& right);
    ~mob();
#pragma endregion

#pragma region private method
private:
    fb::game::life*             find_target();
    bool                        near_target(fb::game::direction& out) const;
#pragma endregion


#pragma region public method
public:
    bool                        action();
    void                        attack();
    uint32_t                    hp_down(uint32_t value, fb::game::object* from = nullptr, bool critical = false);

    uint16_t                    damage_min() const;
    uint16_t                    damage_max() const;
    mob::sizes                  size() const;
    uint32_t                    speed() const;
    
    const std::string&          script_attack() const;
    const std::string&          script_die() const;
    offensive_type              offensive() const;

    uint32_t                    random_damage(fb::game::life& life) const;

    const point16_t&            spawn_point() const;
    void                        spawn_point(uint16_t x, uint16_t y);
    void                        spawn_point(const point16_t point);

    const size16_t&             spawn_size() const;
    void                        spawn_size(uint16_t width, uint16_t height);
    void                        spawn_size(const size16_t size);

    uint64_t                    action_time() const;
    void                        action_time(uint64_t ms);

    uint64_t                    dead_time() const;
    void                        dead_time(uint64_t ms);

    uint32_t                    respawn_time() const;
    void                        respawn_time(uint64_t ms);

    bool                        spawn(uint64_t now);

    fb::game::life*             target() const;
    void                        target(fb::game::life* value);

    const std::vector<drop>&    items() const;
    fb::game::life*             fix();
    void                        AI(uint64_t now);
#pragma endregion
};

} }

#endif // !__MOB_H__
