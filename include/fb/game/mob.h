#ifndef __MOB_H__
#define __MOB_H__

#include <chrono>
#include <fb/game/life.h>
#include <fb/game/item.h>

using namespace std::chrono_literals;

namespace fb { namespace game {

class session;

class rezen
{
private:
    fb::game::context&                      _context;
    const fb::model::mob_spawn&             _model;
    uint16_t                                _count = 0;
    std::chrono::milliseconds               _respawn_time = 0ms;

public:
    rezen(fb::game::context& context, const fb::model::mob_spawn& model);
    ~rezen() = default;

    void                                    decrease();
    void                                    spawn(std::thread::id thread_id);
};

class mob : public life
{
public:
    using model_type = fb::model::mob;

public:
    LUA_PROTOTYPE

public:
    interface listener;

public:
    struct config : fb::game::life::config
    {
    public:
        const bool alive = false;
        rezen* const rezen = nullptr;
    };

private:
    listener*                               _listener       = nullptr;
    std::chrono::milliseconds               _action_time    = 0ms;
    fb::game::rezen*                        _rezen          = nullptr;

    fb::game::life*                         _target         = nullptr;
    lua::context*                           _attack_thread  = nullptr;

public:
    mob(fb::game::context& context, const fb::model::mob& model, const fb::game::mob::config& config);
    mob(const mob& right);
    ~mob();

private:
    fb::game::life*                         find_target();
    bool                                    near_target(DIRECTION& out) const;

public:
    bool                                    action();
    uint32_t                                hp_down(uint32_t value, fb::game::object* from = nullptr, bool critical = false);

    std::chrono::milliseconds               action_time() const;
    void                                    action_time(std::chrono::milliseconds ms);

    fb::game::life*                         target() const;
    void                                    target(fb::game::life* value);

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

} }

#endif // !__MOB_H__
