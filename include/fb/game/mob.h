#ifndef __MOB_H__
#define __MOB_H__

#include <chrono>
#include <fb/game/life.h>
#include <fb/game/item.h>

using namespace std::chrono_literals;

namespace fb { namespace game {

#pragma region forward declaration
class session;
#pragma endregion

class mob : public life
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region forward nested declaration
public:
    interface listener;

public:
    class master;

public:
    struct damage;
    struct drop;
#pragma endregion

#pragma region structure
    struct config : fb::game::life::config
    {
    public:
        const bool alive = false;
    };
#pragma endregion

#pragma region enum
public:
    enum class offensive_type : uint8_t { CONTAINMENT, COUNTER, NONE, NON_MOVE, RUN_AWAY};

    enum class sizes : uint8_t { SMALL = 0x00, LARGE = 0x01 };
#pragma endregion

#pragma region private field
private:
    listener*                               _listener      = nullptr;
    point16_t                               _spawn_point   = point16_t(0, 0);
    size16_t                                _spawn_size    = size16_t(0, 0);
                                                           
    std::chrono::milliseconds               _action_time   = 0ms; // ms
    std::chrono::milliseconds               _dead_time     = 0ms; // ms
    std::chrono::milliseconds               _respawn_time  = 0s; // seconds
                                                           
    fb::game::life*                         _target        = nullptr;
    lua::lua*                               _attack_thread = nullptr;
#pragma endregion

#pragma region constructor / destructor
public:
    mob(fb::game::context& context, const mob::master* master, const fb::game::mob::config& config);
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
protected:
    uint32_t                                handle_calculate_damage(bool critical) const final;
    void                                    handle_attack(fb::game::object* target) final;
    void                                    handle_hit(fb::game::life& you, uint32_t damage, bool critical) final;
    void                                    handle_kill(fb::game::life& you) final;
    void                                    handle_damaged(fb::game::object* from, uint32_t damage, bool critical) final;
    void                                    handle_die(fb::game::object* from) final;

public:
    uint32_t                                handle_exp() const final;
#pragma endregion
};

#pragma region interface
interface mob::listener : public virtual fb::game::life::listener
{
    virtual void                            on_attack(mob& me, object* you) = 0;
    virtual void                            on_hit(mob& me, life& you, uint32_t damage, bool critical) = 0;
    virtual void                            on_kill(mob& me, life& you) = 0;
    virtual void                            on_damaged(mob& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void                            on_die(mob& me, object* you) = 0;
};
#pragma endregion

#pragma region damage
struct mob::damage
{
public:
    uint16_t min, max;

public:
    damage() : damage(0, 0) { }
    damage(uint16_t min, uint16_t max) : min(min), max(max) { }
};
#pragma endregion

#pragma region drop
struct mob::drop
{
public:
    float percentage;
    const fb::game::item::master* item;

public:
    drop() : percentage(0), item(NULL) { }
    drop(const fb::game::item::master* item, float percentage) : item(item), percentage(percentage) { }
    drop(const drop& right) : percentage(right.percentage), item(right.item) { }
};
#pragma endregion

#pragma region nested class
class mob::master : public fb::game::life::master
{
#pragma region structure
public:
struct config : fb::game::life::master::config
{
public:
    const mob::damage                         damage;
    const offensive_type                      offensive;
    const sizes                               size;
    const std::chrono::milliseconds           speed;
    const std::string                         script_attack;
    const std::string                         script_die;
};
#pragma endregion

#pragma region friend
public:
    friend class mob;
#pragma endregion

#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region private field
private:
    std::vector<drop>                       _items;
#pragma endregion

#pragma region public field
public:
    const mob::damage                       damage;
    const offensive_type                    offensive;
    const sizes                             size;
    const std::chrono::milliseconds         speed;
    const std::string                       script_attack, script_die;
    const std::vector<drop>&                items;
#pragma endregion

#pragma region constructor / destructor
public:
    master(const fb::game::mob::master::config& config);
    ~master();
#pragma endregion

#pragma region public method
public:
    object::types                           type() const { return fb::game::object::types::MOB; }

public:
    void                                    push_drop(const drop& drop);
#pragma endregion

#pragma region built-in method
public:
    static int                              builtin_speed(lua_State* lua);
#pragma endregion
};
#pragma endregion

} }

#endif // !__MOB_H__
