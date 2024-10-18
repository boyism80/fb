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
    std::optional<datetime>      _respawn_time;

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
        fb::game::rezen* const rezen = nullptr;
    };

private:
    listener*                               _listener       = nullptr;
    datetime                     _action_time;
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
    const datetime&              action_time() const;
    void                                    action_time(const datetime& dt);

    fb::game::life*                         target() const;
    void                                    target(fb::game::life* value);

    fb::game::life*                         fix();
    void                                    AI(const datetime& now);

    virtual bool                            available() const { return this->alive(); }

protected:
    uint32_t                                on_calculate_damage(bool critical) const final;
    void                                    on_damaged(fb::game::object* from, uint32_t damage, bool critical) final;
    void                                    on_die(fb::game::object* from) final;

public:
    uint32_t                                on_exp() const final;
};

interface mob::listener : public virtual fb::game::life::listener
{
};

} }

#endif // !__MOB_H__
