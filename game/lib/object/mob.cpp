#include <fb/game/session.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/core/thread.h>
#include <fb/game/context.h>

fb::game::mob::mob(fb::game::context& context, const fb::model::mob& model, const fb::game::mob::config& config) : 
    life(context, model, config)
{
    this->visible(config.alive);
    if(config.alive)
    {
        this->hp_up(this->base_hp());
        this->mp_up(this->base_mp());
    }
}

fb::game::mob::mob(const fb::game::mob& right) :
    life(right),
    _spawn_point(right._spawn_point),
    _spawn_size(right._spawn_size),
    _action_time(right._action_time),
    _dead_time(right._dead_time),
    _respawn_time(right._respawn_time),
    _target(right._target)
{ }

fb::game::mob::~mob()
{ }

bool fb::game::mob::action()
{
    this->fix();

    auto& model = this->based<fb::model::mob>();
    if(model.attack_script.empty())
        return false;

    if(this->_attack_thread == nullptr)
    {
        this->_attack_thread = fb::game::lua::get();
        if(this->_attack_thread == nullptr)
            return false;

        this->_attack_thread->from(model.attack_script.c_str())
            .func("on_attack")
            .pushobject(this);

        if(this->_target != nullptr)
            this->_attack_thread->pushobject(this->_target);
        else
            this->_attack_thread->pushnil();
        
        this->_attack_thread->resume(2);
    }

    auto stop = false;
    switch(this->_attack_thread->state())
    {
    case LUA_PENDING:
    case LUA_YIELD:
        stop = true;
        break;

    default:
        stop = this->_attack_thread->toboolean(-1);
        this->_attack_thread = nullptr;
        break;
    }
    
    return stop;
}

uint32_t fb::game::mob::hp_down(uint32_t value, fb::game::object* from, bool critical)
{
    value = fb::game::life::hp_down(value, from, critical);
    if(this->_hp == 0)
        this->visible(false);

    return value;
}

const point16_t& fb::game::mob::spawn_point() const
{
    return this->_spawn_point;
}

void fb::game::mob::spawn_point(uint16_t x, uint16_t y)
{
    this->_spawn_point.x = x;
    this->_spawn_point.y = y;
}

void fb::game::mob::spawn_point(const point16_t point)
{
    this->_spawn_point = point;
}

const size16_t& fb::game::mob::spawn_size() const
{
    return this->_spawn_size;
}

void fb::game::mob::spawn_size(uint16_t width, uint16_t height)
{
    this->_spawn_size.width = width;
    this->_spawn_size.height = height;
}

void fb::game::mob::spawn_size(const size16_t size)
{
    this->_spawn_size = size;
}

std::chrono::milliseconds fb::game::mob::action_time() const
{
    return this->_action_time;
}

void fb::game::mob::action_time(std::chrono::milliseconds ms)
{
    this->_action_time = ms;
}

std::chrono::milliseconds fb::game::mob::dead_time() const
{
    return this->_dead_time;
}

void fb::game::mob::dead_time(std::chrono::milliseconds ms)
{
    this->_dead_time = ms;
}

std::chrono::milliseconds fb::game::mob::respawn_time() const
{
    return this->_respawn_time;
}

void fb::game::mob::respawn_time(std::chrono::milliseconds ms)
{
    this->_respawn_time = ms;
}

bool fb::game::mob::spawn(std::chrono::steady_clock::duration now)
{
    auto& config = fb::config::get();
    if(this->_map->active == false)
        return false;

    if(this->alive())
        return false;

    if(this->_spawn_size.empty())
        return false;

    if((now - this->_dead_time) / 1000 < this->_respawn_time)
        return false;

    this->direction(DIRECTION(std::rand() % 4));
    this->hp_up(this->base_hp());

    while(true)
    {
        point16_t position(this->_spawn_point.x + (std::rand() % this->_spawn_size.width), this->_spawn_point.y + (std::rand() % this->_spawn_size.height));

        if(position.x > this->_map->width()-1 || position.y > this->_map->height()-1)
            continue;

        if(this->_map->blocked(position.x, position.y))
            continue;

        this->position(position, true);
        break;
    }

    this->action_time(std::chrono::duration_cast<std::chrono::milliseconds>(now));
    return true;
}

fb::game::life* fb::game::mob::target() const
{
    return this->_target;
}

void fb::game::mob::target(fb::game::life* value)
{
    this->_target = value;
}

fb::game::life* fb::game::mob::fix()
{
    try
    {
        if(this->_target == nullptr)
            throw nullptr;

        if(this->_target->alive() == false)
            throw nullptr;

        if(this->sight(*this->_target) == false)
            throw nullptr;
    }
    catch(...)
    {
        auto& model = this->based<fb::model::mob>();
        if(model.attack_type == MOB_ATTACK_TYPE::CONTAINMENT)
            this->_target = this->find_target();
        else
            this->_target = nullptr;
    }

    return this->_target;
}

fb::game::life* fb::game::mob::find_target()
{
    auto                    map = this->_map;
    if(map == nullptr)
        return nullptr;

    auto                    min_distance_sqrt = 0xFFFFFFFF;
    for(auto x : this->showings(OBJECT_TYPE::SESSION))
    {
        auto                life = static_cast<fb::game::life*>(x);
        if(life->alive() == false)
            continue;

        auto                distance_sqrt = (uint32_t)std::abs(x->x() - this->x()) * std::abs(x->y() - this->y());
        if(distance_sqrt > min_distance_sqrt)
            continue;

        this->_target = life;
    }

    return this->_target;
}

bool fb::game::mob::near_target(DIRECTION& out) const
{
    for(int i = 0; i < 4; i++)
    {
        auto                direction = DIRECTION(i);
        if(this->side(direction, OBJECT_TYPE::SESSION) != this->_target)
            continue;

        out = direction;
        return true;
    }

    return false;
}

void fb::game::mob::AI(std::chrono::steady_clock::duration now)
{
    try
    {
        auto& model = this->based<fb::model::mob>();
        if(now < this->_action_time + model.speed)
            return;

        // 유효한 타겟이 없으면 고쳐준다.
        auto direction = DIRECTION::BOTTOM;
        if(this->fix() == nullptr)
        {
            this->move(DIRECTION(std::rand() % 4));
        }
        else if(this->near_target(direction))
        {
            this->direction(direction);
            this->attack();
        }
        else
        {
            auto                    x_axis = bool(std::rand()%2);
            if(x_axis)
            {
                if(this->_position.x > this->_target->x() && this->move(DIRECTION::LEFT))   throw nullptr;
                if(this->_position.x < this->_target->x() && this->move(DIRECTION::RIGHT))  throw nullptr;
                if(this->_position.y > this->_target->y() && this->move(DIRECTION::TOP))    throw nullptr;
                if(this->_position.y < this->_target->y() && this->move(DIRECTION::BOTTOM)) throw nullptr;
            }
            else
            {
                if(this->_position.y > this->_target->y() && this->move(DIRECTION::TOP))    throw nullptr;
                if(this->_position.y < this->_target->y() && this->move(DIRECTION::BOTTOM)) throw nullptr;
                if(this->_position.x > this->_target->x() && this->move(DIRECTION::LEFT))   throw nullptr;
                if(this->_position.x < this->_target->x() && this->move(DIRECTION::RIGHT))  throw nullptr;
            }


            // 이동할 수 있는 방향으로 일단 이동한다.
            auto                    random_direction = std::rand() % 4;
            for(int i = 0; i < 4; i++)
            {
                if(this->move(DIRECTION((random_direction + i) % 4)))
                    throw nullptr;
            }
        }
    }
    catch(...)
    {
        
    }

    this->_action_time = std::chrono::duration_cast<std::chrono::milliseconds>(now);
}

uint32_t fb::game::mob::on_calculate_damage(bool critical) const
{
    auto& model = this->based<fb::model::mob>();
    auto difference = model.damage.max - model.damage.min;
    return model.damage.min + (std::rand() % difference);
}

void fb::game::mob::on_attack(fb::game::object* target)
{
    fb::game::life::on_attack(target);

    auto listener = this->get_listener<fb::game::mob>();
    if(listener != nullptr)
        listener->on_attack(*this, target);
}

void fb::game::mob::on_hit(fb::game::life& you, uint32_t damage, bool critical)
{
    fb::game::life::on_hit(you, damage, critical);

    auto listener = this->get_listener<fb::game::mob>();
    if(listener != nullptr)
        listener->on_hit(*this, you, damage, critical);
}

void fb::game::mob::on_kill(fb::game::life& you)
{
    fb::game::life::on_kill(you);

    auto listener = this->get_listener<fb::game::mob>();
    if(listener != nullptr)
        listener->on_kill(*this, you);
}

void fb::game::mob::on_damaged(fb::game::object* from, uint32_t damage, bool critical)
{
    fb::game::life::on_damaged(from, damage, critical);

    auto& model = this->based<fb::model::mob>();
    if(model.attack_type != MOB_ATTACK_TYPE::NONE && from != nullptr && from->is(OBJECT_TYPE::LIFE))
    {
        this->target(static_cast<fb::game::life*>(from));
    }

    auto listener = this->get_listener<fb::game::mob>();
    if(listener != nullptr)
        listener->on_damaged(*this, from, damage, critical);
}

uint32_t fb::game::mob::on_exp() const
{
    auto& model = this->based<fb::model::mob>();
    return model.exp;
}

void fb::game::mob::on_die(fb::game::object* from)
{
    fb::game::life::on_die(from);

    this->dead_time(std::chrono::duration_cast<std::chrono::milliseconds>(fb::thread::now()));

    // 드롭 아이템 떨구기
    auto& model = this->based<fb::model::mob>();
    auto& drop = this->context.model.drop[model.drop];

    for (auto& dsl : drop.dsl)
    {
        switch (dsl.header)
        {
        case DSL::item:
        {
            auto params = fb::model::dsl::item(dsl.params);
            if (std::rand() % 100 > params.percent)
                continue;

            auto item = this->context.model.item[params.id].make(this->context);
            item->map(this->map(), this->position());
        }
            break;
        }
    }

    auto listener = this->get_listener<fb::game::mob>();
    if(listener != nullptr)
        listener->on_die(*this, from);
}