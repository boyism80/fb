#ifndef __MOB_H__
#define __MOB_H__

#include "model/life/life.h"
#include "model/item/item.h"


namespace fb { namespace game {

class session;

class mob : public life
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region listener
public:
interface listener : public virtual fb::game::life::listener
{
    virtual void on_attack(mob& me, object* you) = 0;
    virtual void on_hit(mob& me, life& you, uint32_t damage, bool critical) = 0;
    virtual void on_kill(mob& me, life& you) = 0;
    virtual void on_damaged(mob& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void on_die(mob& me, object* you) = 0;
};
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
    mob::damage                             _damage;
    offensive_type                          _offensive_type;
    sizes                                   _size;
    std::chrono::milliseconds               _speed;
    std::string                             _script_attack, _script_die;
    std::vector<drop>                       _items;
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
    object::types                           type() const { return fb::game::object::types::MOB; }
    object*                                 make(listener* listener) const;

public:
    uint16_t                                damage_min() const;
    void                                    damage_min(uint16_t value);

    uint16_t                                damage_max() const;
    void                                    damage_max(uint16_t value);

    mob::sizes                              size() const;
    void                                    size(mob::sizes value);

    std::chrono::milliseconds               speed() const;
    void                                    speed(std::chrono::milliseconds value);

    const std::string&                      script_attack() const;
    void                                    script_attack(const std::string& value);

    const std::string&                      script_die() const;
    void                                    script_die(const std::string& value);

    offensive_type                          offensive() const;
    void                                    offensive(offensive_type value);

    void                                    dropitem_add(const mob::drop& money);
    void                                    dropitem_add(const fb::game::item::master* item, float percentage);
    const std::vector<drop>&                items() const;
#pragma endregion


#pragma region built-in method
public:
    static int                              builtin_speed(lua_State* lua);
#pragma endregion
};
#pragma endregion


#pragma region private field
private:
    listener*                               _listener;
    point16_t                               _spawn_point;
    size16_t                                _spawn_size;

    std::chrono::milliseconds               _action_time; // ms
    std::chrono::milliseconds               _dead_time; // ms
    std::chrono::milliseconds               _respawn_time; // seconds

    fb::game::life*                         _target;
    lua::lua*                               _attack_thread;
#pragma endregion
    

#pragma region constructor / destructor
public:
    mob(const mob::master* master, listener* listener, bool alive = false);
    mob(const mob& right);
    ~mob();
#pragma endregion

#pragma region private method
private:
    fb::game::life*                         find_target();
    bool                                    near_target(fb::game::direction& out) const;
#pragma endregion


#pragma region public method
public:
    bool                                    action();
    uint32_t                                hp_down(uint32_t value, fb::game::object* from = nullptr, bool critical = false);

    uint16_t                                damage_min() const;
    uint16_t                                damage_max() const;
    mob::sizes                              size() const;
    std::chrono::milliseconds               speed() const;
    
    const std::string&                      script_attack() const;
    const std::string&                      script_die() const;
    offensive_type                          offensive() const;

    const point16_t&                        spawn_point() const;
    void                                    spawn_point(uint16_t x, uint16_t y);
    void                                    spawn_point(const point16_t point);

    const size16_t&                         spawn_size() const;
    void                                    spawn_size(uint16_t width, uint16_t height);
    void                                    spawn_size(const size16_t size);

    std::chrono::milliseconds               action_time() const;
    void                                    action_time(std::chrono::milliseconds ms);

    std::chrono::milliseconds               dead_time() const;
    void                                    dead_time(std::chrono::milliseconds ms);

    std::chrono::milliseconds               respawn_time() const;
    void                                    respawn_time(std::chrono::milliseconds seconds);

    bool                                    spawn(std::chrono::steady_clock::duration now);

    fb::game::life*                         target() const;
    void                                    target(fb::game::life* value);

    const std::vector<drop>&                items() const;
    fb::game::life*                         fix();
    void                                    AI(std::chrono::steady_clock::duration now);

    virtual bool                            available() const { return this->alive(); }
#pragma endregion

#pragma region override method
    uint32_t                                handle_calculate_damage(bool critical) const;
    void                                    handle_attack(fb::game::object* target);
    void                                    handle_hit(fb::game::life& you, uint32_t damage, bool critical);
    void                                    handle_kill(fb::game::life& you);
    void                                    handle_damaged(fb::game::object* from, uint32_t damage, bool critical);
    void                                    handle_die(fb::game::object* from);
#pragma endregion
};

} }

#endif // !__MOB_H__
