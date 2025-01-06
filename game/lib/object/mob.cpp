#include <fb/game/character.h>
#include <fb/game/context.h>
#include <fb/thread.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>

using namespace fb::game;

rezen::rezen(context& context, const fb::model::mob_spawn& model) :
    _context(context),
    _model(model)
{
    this->_respawn_time = fb::model::datetime();
}

void rezen::decrease()
{
    auto now = fb::model::datetime();

    if (!this->_respawn_time.has_value())
        this->_respawn_time = now + this->_model.rezen;
    this->_count = std::max(0, this->_count - 1);
}

void rezen::spawn(std::thread::id thread_id)
{
    if (this->_context.maps.contains(this->_model.parent) == false)
        return;

    auto& map = this->_context.maps[this->_model.parent];
    if (map.active == false)
        return;

    if (map.is_active() == false)
        return;

    auto thread = this->_context.thread(map);
    if (thread == nullptr || thread->id() != thread_id)
        return;

    auto now = fb::model::datetime();
    if (!this->_respawn_time.has_value())
        return;

    if (now < this->_respawn_time)
        return;

    auto spawn_count = this->_model.count - this->_count;
    if (spawn_count < 1)
        return;

    for (int i = 0; i < spawn_count; i++)
    {
        auto mob = this->_context.make<fb::game::mob>(this->_context.model.mob[this->_model.mob],
                                                      fb::game::mob::initial_params{.alive = true, .rezen = this});

        mob->direction(DIRECTION(std::rand() % 4));
        mob->heal(mob->base_hp());

        while (true)
        {
            auto  width    = this->_model.end.x - this->_model.begin.x;
            auto  height   = this->_model.end.y - this->_model.begin.y;
            auto& map      = this->_context.maps[this->_model.parent];
            auto  position = fb::model::point16_t(this->_model.begin.x + (width > 0 ? std::rand() % width : 0),
                                                 this->_model.begin.y + (height > 0 ? std::rand() % height : 0));

            if (position.x > map.width() - 1 || position.y > map.height() - 1)
                continue;

            if (map.blocked(position.x, position.y))
                continue;

            mob->position(position, true);
            std::ignore = mob->map(&map, position);
            break;
        }

        mob->action_time(now);
        mob->visible(true);
    }

    this->_respawn_time.reset();
}

mob::mob(fb::game::context& context, const fb::model::mob& model, const initial_params& params) :
    life(context, model, params),
    _rezen(params.rezen)
{
    this->visible(params.alive);
    if (params.alive)
    {
        this->heal(this->base_hp());
        this->mp_up(this->base_mp());
    }
}

mob::mob(const mob& right) :
    life(right),
    _action_time(right._action_time),
    _target(right._target)
{ }

mob::~mob()
{
    if (this->_rezen != nullptr)
        this->_rezen->decrease();
}

bool mob::action()
{
    this->assert_thread();

    this->fix();

    auto& model = this->based<fb::model::mob>();
    if (model.attack_script.empty())
        return false;

    if (this->_attack_thread == nullptr)
    {
        this->_attack_thread = fb::lua::get();
        if (this->_attack_thread == nullptr)
            return false;

        this->_attack_thread->from(model.attack_script.c_str()).func("on_attack").pushobject(this);

        if (this->_target != nullptr)
            this->_attack_thread->pushobject(this->_target);
        else
            this->_attack_thread->pushnil();

        this->_attack_thread->resume(2);
    }

    auto stop = false;
    switch (this->_attack_thread->state())
    {
    case LUA_PENDING:
    case LUA_YIELD:
        stop = true;
        break;

    default:
        stop                 = this->_attack_thread->toboolean(-1);
        this->_attack_thread = nullptr;
        break;
    }

    return stop;
}

const fb::model::datetime& mob::action_time() const
{
    this->assert_thread();

    return this->_action_time;
}

void mob::action_time(const fb::model::datetime& dt)
{
    this->assert_thread();

    this->_action_time = dt;
}

life* mob::target() const
{
    this->assert_thread();

    return this->_target;
}

void mob::target(life* value)
{
    this->assert_thread();

    this->_target = value;
}

life* mob::fix()
{
    this->assert_thread();

    try
    {
        if (this->_target == nullptr)
            throw nullptr;

        if (this->context.alive(*this->_target) == false)
            throw nullptr;

        if (this->_target->alive() == false)
            throw nullptr;

        if (this->sight(*this->_target) == false)
            throw nullptr;
    }
    catch (...)
    {
        auto& model = this->based<fb::model::mob>();
        if (model.attack_type == MOB_ATTACK_TYPE::CONTAINMENT)
            this->_target = this->find_target();
        else
            this->_target = nullptr;
    }

    return this->_target;
}

life* mob::find_target()
{
    this->assert_thread();

    auto map = this->_map;
    if (map == nullptr)
        return nullptr;

    auto min_distance_sqrt = 0xFFFFFFFF;
    for (auto x : this->showings(OBJECT_TYPE::CHARACTER))
    {
        auto life = static_cast<fb::game::life*>(x);
        if (life->alive() == false)
            continue;

        auto distance_sqrt = (uint32_t)std::abs(x->x() - this->x()) * std::abs(x->y() - this->y());
        if (distance_sqrt > min_distance_sqrt)
            continue;

        this->_target = life;
    }

    return this->_target;
}

bool mob::near_target(DIRECTION& out) const
{
    this->assert_thread();

    for (int i = 0; i < 4; i++)
    {
        auto direction = DIRECTION(i);
        if (this->side(direction, OBJECT_TYPE::CHARACTER) != this->_target)
            continue;

        out = direction;
        return true;
    }

    return false;
}

void mob::AI(const fb::model::datetime& now)
{
    this->assert_thread();

    try
    {
        auto& model = this->based<fb::model::mob>();
        if (now < this->_action_time + model.speed)
            return;

        // 유효한 타겟이 없으면 고쳐준다.
        auto direction = DIRECTION::BOTTOM;
        if (this->fix() == nullptr)
        {
            this->move(DIRECTION(std::rand() % 4));
        }
        else if (this->near_target(direction))
        {
            this->direction(direction);
            this->attack();
        }
        else
        {
            auto x_axis = bool(std::rand() % 2);
            if (x_axis)
            {
                if (this->_position.x > this->_target->x() && this->move(DIRECTION::LEFT))
                    throw nullptr;
                if (this->_position.x < this->_target->x() && this->move(DIRECTION::RIGHT))
                    throw nullptr;
                if (this->_position.y > this->_target->y() && this->move(DIRECTION::TOP))
                    throw nullptr;
                if (this->_position.y < this->_target->y() && this->move(DIRECTION::BOTTOM))
                    throw nullptr;
            }
            else
            {
                if (this->_position.y > this->_target->y() && this->move(DIRECTION::TOP))
                    throw nullptr;
                if (this->_position.y < this->_target->y() && this->move(DIRECTION::BOTTOM))
                    throw nullptr;
                if (this->_position.x > this->_target->x() && this->move(DIRECTION::LEFT))
                    throw nullptr;
                if (this->_position.x < this->_target->x() && this->move(DIRECTION::RIGHT))
                    throw nullptr;
            }

            // 이동할 수 있는 방향으로 일단 이동한다.
            auto random_direction = std::rand() % 4;
            for (int i = 0; i < 4; i++)
            {
                if (this->move(DIRECTION((random_direction + i) % 4)))
                    throw nullptr;
            }
        }
    }
    catch (...)
    { }

    this->_action_time = now;
}

bool mob::available() const
{
    this->assert_thread();

    return this->alive();
}

uint32_t mob::damage(uint32_t value, object* from, bool critical)
{
    this->assert_thread();

    auto result = life::damage(value, from, critical);
    if (from == nullptr)
        return result;

    if (!this->alive())
    {
        this->kill(from, DESTROY_TYPE::DEAD);
        return result;
    }

    auto& model = this->based<fb::model::mob>();
    switch (model.attack_type)
    {
    case MOB_ATTACK_TYPE::NONE:
        break;

    case MOB_ATTACK_TYPE::RUN_AWAY:
        break;

    default:
    {
        if (from->is(OBJECT_TYPE::LIFE))
        {
            if (this->_target == nullptr)
            {
                this->target(static_cast<life*>(from));
            }
            else
            {
                // TODO: 가장 최근에 공격한 대상이 일정 시간 이상 공격하지 않았으면
                // 타겟을 변경한다.
            }
        }
    }
    break;
    }

    return result;
}

uint32_t mob::auto_attack_damage(MOB_SIZE size) const
{
    this->assert_thread();

    auto& model      = this->based<fb::model::mob>();
    auto  difference = model.damage.max - model.damage.min;
    return model.damage.min + (std::rand() % difference);
}

void mob::kill(object* from, DESTROY_TYPE destroy_type)
{
    life::kill(from, destroy_type);
    this->destroy(destroy_type);
}

void mob::drop_items()
{
    this->assert_thread();

    // 드롭 아이템 떨구기
    auto& model = this->based<fb::model::mob>();
    if (model.drop.empty())
        return;

    auto& drop = this->context.model.drop[model.drop];
    for (auto& dsl : drop.dsl)
    {
        switch (dsl.header)
        {
        case DSL::item:
        {
            auto params = fb::model::dsl::item(dsl.params);
            auto random = std::rand() % 100;
            if (random > (int)params.percent)
                continue;

            auto item   = this->context.model.item[params.id].make(this->context);
            std::ignore = item->map(this->map(), this->position());
        }
        break;
        }
    }
}

void mob::assert_thread() const
{
    if (this->_map != nullptr)
        object::assert_thread();
    else
        return;
}