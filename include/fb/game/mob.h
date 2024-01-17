#ifndef __MOB_H__
#define __MOB_H__

#include <chrono>
#include <fb/game/life.h>
#include <fb/game/item.h>

using namespace std::chrono_literals;

namespace fb { namespace game {

class session;

class mob : public life
{
public:
    LUA_PROTOTYPE

public:
    interface listener;

public:
    class model;

public:
    struct damage;
    struct drop;

    struct config : fb::game::life::config
    {
    public:
        const bool alive = false;
    };

public:
    enum class offensive_type : uint8_t { CONTAINMENT, COUNTER, NONE, NON_MOVE, RUN_AWAY};

    enum class sizes : uint8_t { SMALL = 0x00, LARGE = 0x01 };

private:
    listener*                               _listener      = nullptr;
    point16_t                               _spawn_point   = point16_t(0, 0);
    size16_t                                _spawn_size    = size16_t(0, 0);
                                                           
    std::chrono::milliseconds               _action_time   = 0ms; // ms
    std::chrono::milliseconds               _dead_time     = 0ms; // ms
    std::chrono::milliseconds               _respawn_time  = 0s; // seconds
                                                           
    fb::game::life*                         _target        = nullptr;
    lua::context*                           _attack_thread = nullptr;

public:
    mob(fb::game::context& context, const mob::model* model, const fb::game::mob::config& config);
    mob(const mob& right);
    ~mob();

private:
    fb::game::life*                         find_target();
    bool                                    near_target(fb::game::DIRECTION_TYPE& out) const;

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

protected:
    uint32_t                                on_calculate_damage(bool critical) const final;
    void                                    on_attack(fb::game::object* target) final;
    void                                    on_hit(fb::game::life& you, uint32_t damage, bool critical) final;
    void                                    on_kill(fb::game::life& you) final;
    void                                    on_damaged(fb::game::object* from, uint32_t damage, bool critical) final;
    void                                    on_die(fb::game::object* from) final;

public:
    uint32_t                                on_exp() const final;
};

interface mob::listener : public virtual fb::game::life::listener
{
    virtual void                            on_attack(mob& me, object* you) = 0;
    virtual void                            on_hit(mob& me, life& you, uint32_t damage, bool critical) = 0;
    virtual void                            on_kill(mob& me, life& you) = 0;
    virtual void                            on_damaged(mob& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void                            on_die(mob& me, object* you) = 0;
};

struct mob::damage
{
public:
    uint16_t min, max;

public:
    damage() : damage(0, 0) { }
    damage(uint16_t min, uint16_t max) : min(min), max(max) { }
};

struct mob::drop
{
public:
    float percentage;
    const fb::game::item::model* item;

public:
    drop() : percentage(0), item(NULL) { }
    drop(const fb::game::item::model* item, float percentage) : item(item), percentage(percentage) { }
    drop(const drop& right) : percentage(right.percentage), item(right.item) { }
};

class mob::model : public fb::game::life::model
{
public:
struct config : fb::game::life::model::config
{
public:
    const mob::damage                         damage;
    const offensive_type                      offensive;
    const sizes                               size;
    const std::chrono::milliseconds           speed;
    const std::string                         script_attack;
    const std::string                         script_die;
};

public:
    friend class mob;

public:
    LUA_PROTOTYPE

private:
    std::vector<drop>                       _items;

public:
    const mob::damage                       damage;
    const offensive_type                    offensive;
    const sizes                             size;
    const std::chrono::milliseconds         speed;
    const std::string                       script_attack, script_die;
    const std::vector<drop>&                items;

public:
    model(const fb::game::mob::model::config& config);
    ~model();

public:
    object::types                           type() const { return fb::game::object::types::MOB; }

public:
    void                                    push_drop(const drop& drop);

public:
    static int                              builtin_speed(lua_State* lua);
};

} }

#endif // !__MOB_H__
