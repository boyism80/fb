#include "model/session/session.h"
#include "model/map/map.h"
#include "model/listener/listener.h"
#include "mob.h"

fb::game::mob::master::master(const std::string& name, uint16_t look, uint8_t color, uint32_t hp, uint32_t mp) : 
    fb::game::life::master(name, look, color, hp, mp),
    _damage(0, 0),
    _offensive_type(offensive_type::NONE),
    _size(sizes::SMALL),
    _speed(1000)
{ }

fb::game::mob::master::master(const life::master& master) :
    life::master(master),
    _damage(0, 0),
    _offensive_type(offensive_type::NONE),
    _size(sizes::SMALL),
    _speed(1000)
{}

fb::game::mob::master::~master()
{}

fb::game::object* fb::game::mob::master::make(listener* listener) const
{
    return new mob(this, listener);
}

uint16_t fb::game::mob::master::damage_min() const
{
    return this->_damage.min;
}

void fb::game::mob::master::damage_min(uint16_t value)
{
    this->_damage.min = value;
}

uint16_t fb::game::mob::master::damage_max() const
{
    return this->_damage.max;
}

void fb::game::mob::master::damage_max(uint16_t value)
{
    this->_damage.max = value;
}

fb::game::mob::sizes fb::game::mob::master::size() const
{
    return this->_size;
}

void fb::game::mob::master::size(mob::sizes value)
{
    this->_size = value;
}

uint32_t fb::game::mob::master::speed() const
{
    return this->_speed;
}

void fb::game::mob::master::speed(uint32_t value)
{
    this->_speed = value;
}

const std::string& fb::game::mob::master::script_attack() const
{
    return this->_script_attack;
}

void fb::game::mob::master::script_attack(const std::string& value)
{
    this->_script_attack = value;
}

const std::string& fb::game::mob::master::script_die() const
{
    return this->_script_die;
}

void fb::game::mob::master::script_die(const std::string& value)
{
    this->_script_die = value;
}

fb::game::mob::offensive_type fb::game::mob::master::offensive() const
{
    return this->_offensive_type;
}

void fb::game::mob::master::offensive(offensive_type value)
{
    this->_offensive_type = value;
}

void fb::game::mob::master::dropitem_add(const mob::drop& money)
{
    this->_items.push_back(money);
}

void fb::game::mob::master::dropitem_add(const fb::game::item::master* item, float percentage)
{
    this->_items.push_back(drop(item, percentage));
}

const std::vector<fb::game::mob::drop>& fb::game::mob::master::items() const
{
    return this->_items;
}

int fb::game::mob::master::builtin_speed(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto mob = thread->touserdata<fb::game::mob::master>(1);
    if(mob == nullptr)
        return 0;

    lua_pushinteger(lua, mob->_speed);
    return 1;
}



fb::game::mob::mob(const mob::master* master, listener* listener, bool alive) : 
    life(master, listener),
    _listener(listener),
    _action_time(0),
    _dead_time(0),
    _respawn_time(0),
    _target(nullptr),
    _attack_thread(nullptr)
{
    this->visible(alive);
    if(alive)
    {
        this->hp_up(this->base_hp());
        this->mp_up(this->base_mp());
    }
}

fb::game::mob::mob(const mob& right) :
    life(right),
    _spawn_point(right._spawn_point),
    _spawn_size(right._spawn_size),
    _action_time(right._action_time),
    _dead_time(right._dead_time),
    _respawn_time(right._respawn_time),
    _target(right._target)
{}

fb::game::mob::~mob()
{
    if(this->_attack_thread != nullptr)
        delete this->_attack_thread;
}

bool fb::game::mob::action()
{
    this->fix();

    auto& script = this->script_attack();
    if(script.empty())
        return false;

    if(this->_attack_thread == nullptr)
    {
        this->_attack_thread = new lua::thread("scripts/mob/%s.lua", script.c_str());
        this->_attack_thread->func("handle_attack").pushobject(this);

        if(this->_target != nullptr)
            this->_attack_thread->pushobject(this->_target);
        else
            this->_attack_thread->pushnil();
        
        this->_attack_thread->resume(2);
    }

    auto stop = false;
    if(this->_attack_thread->state() == LUA_PENDING)
    {
        stop = true;
    }
    else
    {
        stop = this->_attack_thread->toboolean(-1);

        delete this->_attack_thread;
        this->_attack_thread = nullptr;
    }

    return stop;
}

void fb::game::mob::attack()
{
    if(this->alive() == false)
        return;

    auto front = this->forward(object::types::UNKNOWN);

    if(front == nullptr)
        return;

    if(front->is(object::types::LIFE) == false)
        return;

    if(static_cast<life*>(front)->alive() == false)
        return;

    auto damage = this->random_damage(*static_cast<fb::game::life*>(front));
    if(this->_listener != nullptr)
        this->_listener->on_attack(*this, front, damage, false);
}

uint32_t fb::game::mob::hp_down(uint32_t value, fb::game::object* from, bool critical)
{
    value = fb::game::life::hp_down(value, from, critical);
    if(this->_listener != nullptr)
        this->_listener->on_damage(*this, from, value, critical);

    if(this->_hp == 0)
    {
        if(this->_listener != nullptr)
            this->_listener->on_die(*this);

        this->visible(false);
    }
    return value;
}

uint16_t fb::game::mob::damage_min() const
{
    return static_cast<const master*>(this->_master)->_damage.min;
}

uint16_t fb::game::mob::damage_max() const
{
    return static_cast<const master*>(this->_master)->_damage.max;
}

fb::game::mob::sizes fb::game::mob::size() const
{
    return static_cast<const master*>(this->_master)->_size;
}

uint32_t fb::game::mob::speed() const
{
    return static_cast<const master*>(this->_master)->_speed;
}

const std::string& fb::game::mob::script_attack() const
{
    return static_cast<const master*>(this->_master)->_script_attack;
}

const std::string& fb::game::mob::script_die() const
{
    return static_cast<const master*>(this->_master)->_script_die;
}

fb::game::mob::offensive_type fb::game::mob::offensive() const
{
    return static_cast<const master*>(this->_master)->_offensive_type;
}

uint32_t fb::game::mob::random_damage(fb::game::life& life) const
{
    uint32_t difference = std::abs(this->damage_max() - this->damage_min());
    uint32_t random_damage = this->damage_min() + (std::rand() % difference);

    return fb::game::life::random_damage(random_damage, life);
}

const fb::game::point16_t& fb::game::mob::spawn_point() const
{
    return this->_spawn_point;
}

void fb::game::mob::spawn_point(uint16_t x, uint16_t y)
{
    this->_spawn_point.x = x;
    this->_spawn_point.y = y;
}

void fb::game::mob::spawn_point(const fb::game::point16_t point)
{
    this->_spawn_point = point;
}

const fb::game::size16_t& fb::game::mob::spawn_size() const
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

uint64_t fb::game::mob::action_time() const
{
    return this->_action_time;
}

void fb::game::mob::action_time(uint64_t ms)
{
    this->_action_time = ms;
}

uint64_t fb::game::mob::dead_time() const
{
    return this->_dead_time;
}

void fb::game::mob::dead_time(uint64_t ms)
{
    this->_dead_time = ms;
}

uint32_t fb::game::mob::respawn_time() const
{
    return this->_respawn_time;
}

void fb::game::mob::respawn_time(uint64_t ms)
{
    this->_respawn_time = (uint32_t)ms;
}

bool fb::game::mob::spawn(uint64_t now)
{
    auto& config = fb::config::get();
    if(config["id"].asString() != this->_map->host)
        return false;

    if(this->alive())
        return false;

    if(this->_spawn_size.empty())
        return false;

    if((now - this->_dead_time) / 1000 < this->_respawn_time)
        return false;

    this->direction(fb::game::direction(std::rand() % 4));
    this->hp_up(this->base_hp());

    while(true)
    {
        point16_t position(this->_spawn_point.x + (std::rand() % this->_spawn_size.width), this->_spawn_point.y + (std::rand() % this->_spawn_size.height));

        if(position.x > this->_map->width()-1 || position.y > this->_map->height()-1)
            continue;

        if(this->_map->blocked(position.x, position.y))
            continue;

        this->position(position);
        break;
    }

    this->action_time(now);
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
        if(this->offensive() == mob::offensive_type::CONTAINMENT)
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
    for(auto x : this->showings(fb::game::object::types::SESSION))
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

const std::vector<fb::game::mob::drop>& fb::game::mob::items() const
{
    return static_cast<const master*>(this->_master)->items();
}

bool fb::game::mob::near_target(fb::game::direction& out) const
{
    for(int i = 0; i < 4; i++)
    {
        auto                direction = fb::game::direction(i);
        if(this->side(direction, fb::game::object::SESSION) != this->_target)
            continue;

        out = direction;
        return true;
    }

    return false;
}

void fb::game::mob::AI(uint64_t now)
{
    try
    {
        if(now < this->_action_time + this->speed())
            return;

        // 유효한 타겟이 없으면 고쳐준다.
        auto direction = fb::game::direction::BOTTOM;
        if(this->fix() == nullptr)
        {
            this->move(fb::game::direction(std::rand() % 4));
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
                if(this->_position.x > this->_target->x() && this->move(direction::LEFT))   throw nullptr;
                if(this->_position.x < this->_target->x() && this->move(direction::RIGHT))  throw nullptr;
                if(this->_position.y > this->_target->y() && this->move(direction::TOP))    throw nullptr;
                if(this->_position.y < this->_target->y() && this->move(direction::BOTTOM)) throw nullptr;
            }
            else
            {
                if(this->_position.y > this->_target->y() && this->move(direction::TOP))    throw nullptr;
                if(this->_position.y < this->_target->y() && this->move(direction::BOTTOM)) throw nullptr;
                if(this->_position.x > this->_target->x() && this->move(direction::LEFT))   throw nullptr;
                if(this->_position.x < this->_target->x() && this->move(direction::RIGHT))  throw nullptr;
            }


            // 이동할 수 있는 방향으로 일단 이동한다.
            auto                    random_direction = std::rand() % 4;
            for(int i = 0; i < 4; i++)
            {
                if(this->move(fb::game::direction((random_direction + i) % 4)))
                    throw nullptr;
            }
        }
    }
    catch(...)
    {
        
    }

    this->_action_time = now;
}