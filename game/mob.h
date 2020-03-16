#ifndef __MOB_H__
#define __MOB_H__

#include "object.h"
#include "item.h"


namespace fb { namespace game {

class session;

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
        const fb::game::item::core* item;

    public:
        _drop() : percentage(0), item(NULL) {}
        _drop(const fb::game::item::core* item, float percentage) : item(item), percentage(percentage) {}
        _drop(const _drop& right) : percentage(right.percentage), item(right.item) {}
    } money;
#pragma endregion


#pragma region enum
public:
    enum offensive_type : uint8_t { CONTAINMENT, COUNTER, NONE, NON_MOVE, RUN_AWAY};

    enum sizes : uint8_t { SMALL = 0x00, LARGE = 0x01 };
#pragma endregion


#pragma region core class
public:
class core : public fb::game::life::core
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
    std::vector<money>          _items;
#pragma endregion


#pragma region friend
public:
    friend class mob;
#pragma endregion


#pragma region constructor / destructor
public:
    core(const std::string& name, uint16_t look, uint8_t color, uint32_t hp, uint32_t mp);
    core(const life::core& core);
    ~core();
#pragma endregion


#pragma region public method
public:
    object::types               type() const { return fb::game::object::types::MOB; }
    object*                     make() const;

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

    void                        dropitem_add(const mob::money& money);
    void                        dropitem_add(const fb::game::item::core* item, float percentage);
    const std::vector<money>&   items() const;
#pragma endregion


#pragma region built-in method
public:
	static int				    builtin_speed(lua_State* lua);
#pragma endregion
};
#pragma endregion


#pragma region private field
private:
    point16_t                   _spawn_point;
    size16_t                    _spawn_size;

    uint64_t                    _action_time; // ms
    uint64_t                    _dead_time; // ms
    uint32_t                    _respawn_time; // seconds

    fb::game::session*          _target;
#pragma endregion
    

#pragma region constructor / destructor
public:
    mob(const mob::core* core);
    mob(const mob& right);
    ~mob();
#pragma endregion


#pragma region public method
public:
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

    fb::game::session*          target() const;
    void                        target(fb::game::session* value);
    fb::game::session*          autoset_target();

    const std::vector<money>&   items() const;
#pragma endregion
};

} }

#endif // !__MOB_H__
