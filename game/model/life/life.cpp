#include "model/acceptor/acceptor.game.h"
#include "model/listener/listener.h"
#include "model/map/map.h"
#include "life.h"

fb::game::life::master::master(const std::string& name, uint16_t look, uint8_t color, uint32_t hp, uint32_t mp) : 
    object::master(name, look, color),
    _hp(hp), _mp(mp),
    _experience(0)
{}


fb::game::life::master::master(const master& master, uint32_t hp, uint32_t mp) : 
    fb::game::object::master(master),
    _hp(hp), _mp(mp),
    _experience(0)
{
}

fb::game::life::master::master(const master& right) : 
    fb::game::object::master(right),
    _hp(right._hp), _mp(right._mp),
    _experience(right._experience)
{
}

fb::game::life::master::~master()
{
}

uint32_t fb::game::life::master::hp() const
{
    return this->_hp;
}

void fb::game::life::master::hp(uint32_t value)
{
    this->_hp = value;
}

uint32_t fb::game::life::master::mp() const
{
    return this->_mp;
}

void fb::game::life::master::mp(uint32_t value)
{
    this->_mp = value;
}

uint32_t fb::game::life::master::experience() const
{
    return this->_experience;
}

void fb::game::life::master::experience(uint32_t value)
{
    this->_experience = value;
}

uint32_t fb::game::life::master::defensive_physical() const
{
    return this->_defensive.physical;
}

void fb::game::life::master::defensive_physical(uint8_t value)
{
    this->_defensive.physical = value;
}

uint32_t fb::game::life::master::defensive_magical() const
{
    return this->_defensive.magical;
}

void fb::game::life::master::defensive_magical(uint8_t value)
{
    this->_defensive.magical = value;
}

fb::game::object* fb::game::life::master::make(listener* listener) const
{
    return new life(this, listener);
}

int fb::game::life::master::builtin_hp(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    auto object = thread->touserdata<fb::game::life::master>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushinteger(object->_hp);
    return 1;
}

int fb::game::life::master::builtin_mp(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    auto object = thread->touserdata<fb::game::life::master>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushinteger(object->_mp);
    return 1;
}


fb::game::life::life(const master* master, listener* listener) : 
    object(master, listener),
    _listener(listener),
    _hp(0),
    _mp(0),
    _condition(fb::game::condition::NONE),
    spells(*this, listener)
{
}

fb::game::life::life(master* master, listener* listener, uint32_t id, uint32_t hp, uint32_t mp, uint32_t exp) : 
    object(master, listener, id),
    _listener(listener),
    _hp(hp),
    _mp(mp),
    _condition(fb::game::condition::NONE),
    spells(*this, listener)
{
}

fb::game::life::life(const fb::game::object& object, listener* listener, uint32_t hp, uint32_t mp, uint32_t exp) : 
    fb::game::object(object),
    _listener(listener),
    _hp(hp),
    _mp(mp),
    _condition(fb::game::condition::NONE),
    spells(*this, listener)
{
}


fb::game::life::~life()
{
}

uint32_t fb::game::life::random_damage(uint32_t value, const fb::game::life& life) const
{
    uint32_t Xrate = life.direction() == this->direction() ? 2 : 1;
    uint32_t n = (100 - life.defensive_physical()) / 10;
    float defensive_percent = -125 + (n * (2*14.75f - (n-1)/2.0f))/2.0f;
    uint32_t random_damage = value - uint32_t(defensive_percent * (value/100.0f));

    return random_damage * Xrate;
}

uint32_t fb::game::life::hp_up(uint32_t value, fb::game::object* from)
{
    value = std::min(value, this->base_hp() - this->_hp);
    this->hp(this->_hp + value);
    if(this->_listener != nullptr)
        this->_listener->on_heal_hp(*this, value, from);

    return value;
}

uint32_t fb::game::life::hp_down(uint32_t value, fb::game::object* from, bool critical)
{
    value = std::min(value, this->_hp);
    this->hp(this->_hp - value);
    if(this->_listener != nullptr)
        this->_listener->on_damage(*this, from, value, critical);

    if(this->_hp == 0 && this->_listener != nullptr)
        this->_listener->on_die(*this);

    return value;
}

uint32_t fb::game::life::mp_up(uint32_t value, fb::game::object* from)
{
    value = std::min(value, this->base_mp() - this->_mp);
    this->mp(this->_mp + value);
    if(this->_listener != nullptr)
        this->_listener->on_heal_mp(*this, value, from);

    return value;
}

uint32_t fb::game::life::mp_down(uint32_t value, fb::game::object* from)
{
    value = std::min(value, this->_mp);
    this->mp(this->_mp - value);

    return value;
}

void fb::game::life::attack()
{
    if(this->_map == nullptr)
        return;

    auto front = this->forward(object::types::UNKNOWN);

    // TODO: 몬스터 데미지 공식 적용
    if(this->_listener != nullptr)
        this->_listener->on_attack(*this, front, 1, false);
}

uint32_t fb::game::life::hp() const
{
    return this->_hp;
}

void fb::game::life::hp(uint32_t value)
{
    auto before = this->_hp;
    this->_hp = value;
    if(this->_listener != nullptr)
    {
        if(this->is(fb::game::object::types::SESSION))
            this->_listener->on_updated(static_cast<fb::game::session&>(*this), fb::game::state_level::LEVEL_MIDDLE);
        this->_listener->on_hp(*this, before, this->_hp);
    }
}

uint32_t fb::game::life::mp() const
{
    return this->_mp;
}

void fb::game::life::mp(uint32_t value)
{
    auto before = this->_mp;
    this->_mp = value;
    if(this->_listener != nullptr)
    {
        if(this->is(fb::game::object::types::SESSION))
            this->_listener->on_updated(static_cast<fb::game::session&>(*this), fb::game::state_level::LEVEL_MIDDLE);
        this->_listener->on_mp(*this, before, this->_hp);
    }
}

uint32_t fb::game::life::base_hp() const
{
    return static_cast<const master*>(this->_master)->_hp;
}

uint32_t fb::game::life::base_mp() const
{
    return static_cast<const master*>(this->_master)->_mp;
}

uint32_t fb::game::life::experience() const
{
    return static_cast<const master*>(this->_master)->_experience;
}

uint32_t fb::game::life::defensive_physical() const
{
    return static_cast<const master*>(this->_master)->_defensive.physical;
}

uint32_t fb::game::life::defensive_magical() const
{
    return static_cast<const master*>(this->_master)->_defensive.magical;
}

fb::game::condition fb::game::life::condition() const
{
    return this->_condition;
}

fb::game::condition fb::game::life::condition_add(fb::game::condition value)
{
    this->_condition = fb::game::condition(this->_condition | value);
    return this->_condition;
}

fb::game::condition fb::game::life::condition_remove(fb::game::condition value)
{
    this->_condition = fb::game::condition(this->_condition & ~value);
    return this->_condition;
}

bool fb::game::life::condition_contains(fb::game::condition value) const
{
    return uint32_t(this->_condition) & uint32_t(value);
}

bool fb::game::life::alive() const
{
    return this->_hp != 0;
}

void fb::game::life::kill()
{
    this->_hp = 0;
    if(this->_listener != nullptr)
        this->_listener->on_hide(*this);
}

bool fb::game::life::active(fb::game::spell& spell, const std::string& message)
{
    lua::thread             thread;
    thread.from("scripts/spell/%s.lua", spell.cast().c_str())
        .func("handle_spell");

    if(spell.type() != fb::game::spell::types::INPUT)
        return false;

    thread.pushobject(this)
        .pushobject(spell)
        .pushstring(message)
        .resume(3);
    return true;
}

bool fb::game::life::active(fb::game::spell& spell, uint32_t fd)
{
    if(this->_map == nullptr)
        return false;

    auto to = this->_map->objects[fd];
    if(to == nullptr)
        return false;

    return this->active(spell, *to);
}

bool fb::game::life::active(fb::game::spell& spell, fb::game::object& to)
{
    lua::thread             thread;
    thread.from("scripts/spell/%s.lua", spell.cast().c_str())
        .func("handle_spell");

    if(spell.type() != fb::game::spell::types::TARGET)
        return false;

    auto                map = this->map();
    if(map == nullptr)
        return false;

    if(to.map() != this->map())
        return false;

    if(this->sight(to) == false)
        return true;

    thread.pushobject(this)
        .pushobject(spell)
        .pushobject(&to)
        .resume(3);
    return true;
}

bool fb::game::life::active(fb::game::spell& spell)
{
    lua::thread             thread;
    thread.from("scripts/spell/%s.lua", spell.cast().c_str())
        .func("handle_spell");

    if(spell.type() != fb::game::spell::types::NORMAL)
        return false;

    thread.pushobject(this)
        .pushobject(spell)
        .resume(2);
    return true;
}

int fb::game::life::builtin_hp(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || acceptor->exists(*object) == false)
        return 0;
    

    if(argc == 1)
    {
        thread->pushinteger(object->hp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        object->hp(value);
        return 0;
    }
}

int fb::game::life::builtin_mp(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || acceptor->exists(*object) == false)
        return 0;
    

    if(argc == 1)
    {
        thread->pushinteger(object->mp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        object->mp(value);
        return 0;
    }
}

int fb::game::life::builtin_base_hp(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || acceptor->exists(*object) == false)
        return 0;
    
    auto master = object->based<life::master>();

    thread->pushinteger(master->_hp);
    return 1;
}

int fb::game::life::builtin_base_mp(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || acceptor->exists(*object) == false)
        return 0;
    
    auto master = object->based<life::master>();

    thread->pushinteger(master->_mp);
    return 1;
}

int fb::game::life::builtin_hp_inc(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || acceptor->exists(*object) == false)
        return 0;
    
    auto value = (uint32_t)thread->tointeger(2);

    object->hp_up(value);
    return 0;
}

int fb::game::life::builtin_hp_dec(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || acceptor->exists(*object) == false)
        return 0;
    
    auto value = (uint32_t)thread->tointeger(2);

    object->hp_down(value);
    return 0;
}

int fb::game::life::builtin_mp_inc(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || acceptor->exists(*object) == false)
        return 0;
    
    auto value = (uint32_t)thread->tointeger(2);

    object->mp_up(value);
    return 0;
}

int fb::game::life::builtin_mp_dec(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || acceptor->exists(*object) == false)
        return 0;
    
    auto value = (uint32_t)thread->tointeger(2);

    object->mp_down(value);
    return 0;
}

int fb::game::life::builtin_action(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto life = thread->touserdata<fb::game::life>(1);
    if(life == nullptr || acceptor->exists(*life) == false)
        return 0;
    
    auto action = thread->tointeger(2);
    auto duration = argc < 3 ? fb::game::duration::DURATION_SPELL : thread->tointeger(3);
    auto sound = argc < 4 ? (uint8_t)0x00 : (uint8_t)thread->tointeger(4);

    acceptor->send(*life, fb::protocol::game::response::life::action(*life, fb::game::action(action), fb::game::duration(duration), sound), acceptor::scope::PIVOT);
    return 0;
}

int fb::game::life::builtin_spell(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto life = thread->touserdata<fb::game::life>(1);
    if(life == nullptr || acceptor->exists(*life) == false)
        return 0;
    
    auto index = (int)thread->tointeger(2);

    life->spells[index]->to_lua(lua);
    return 1;
}

int fb::game::life::builtin_damage(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto me = thread->touserdata<fb::game::life>(1);
    if(me == nullptr || acceptor->exists(*me) == false)
        return 0;
    
    auto you = thread->touserdata<fb::game::life>(2);
    if(you == nullptr || acceptor->exists(*you) == false)
        return 0;

    auto damage = (uint32_t)thread->tointeger(3);

    me->hp_down(damage, you, false);
    thread->pushboolean(you->visible());
    return 1;
}