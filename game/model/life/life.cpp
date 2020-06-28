#include "life.h"
#include <model/map/map.h>
#include <model/acceptor/acceptor.game.h>
#include <model/listener/listener.h>

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
    auto object = *(fb::game::life::master**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, object->_hp);
    return 1;
}

int fb::game::life::master::builtin_mp(lua_State* lua)
{
    auto object = *(fb::game::life::master**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, object->_mp);
    return 1;
}


fb::game::life::life(const master* master, listener* listener) : 
    object(master, listener),
    _listener(listener),
    _hp(0),
    _mp(0),
    _condition(fb::game::condition::NONE),
    spells(*this)
{
}

fb::game::life::life(master* master, listener* listener, uint32_t id, uint32_t hp, uint32_t mp, uint32_t exp) : 
    object(master, listener, id),
    _listener(listener),
    _hp(hp),
    _mp(mp),
    _condition(fb::game::condition::NONE),
    spells(*this)
{
}

fb::game::life::life(const fb::game::object& object, listener* listener, uint32_t hp, uint32_t mp, uint32_t exp) : 
    object(object),
    _listener(listener),
    _hp(hp),
    _mp(mp),
    _condition(fb::game::condition::NONE),
    spells(*this)
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

    auto front = this->forward_object(object::types::UNKNOWN);

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
        this->_listener->on_hp(*this, before, this->_hp);
}

uint32_t fb::game::life::mp() const
{
    return this->_mp;
}

void fb::game::life::mp(uint32_t value)
{
    this->_mp = value;
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

fb::ostream fb::game::life::make_action_stream(fb::game::action action, fb::game::duration duration, uint8_t sound) const
{
    fb::ostream             ostream;
    ostream.write_u8(0x1A)
        .write_u32(this->id())
        .write_u8(action) // type
        .write_u16(duration) // duration
        .write_u8(sound); // sound

    return ostream;
}

fb::ostream fb::game::life::make_show_hp_stream(uint32_t random_damage, bool critical) const
{
    fb::ostream             ostream;
    uint8_t                 percentage = uint8_t((this->_hp / float(this->base_hp())) * 100);

    ostream.write_u8(0x13)
        .write_u32(this->id())
        .write_u8(critical)
        .write_u8(percentage)
        .write_u32(random_damage)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::life::make_die_stream() const
{
    fb::ostream             ostream;

    ostream.write_u8(0x5F)
        .write_u32(this->id())
        .write_u8(0x00);

    return ostream;
}

int fb::game::life::builtin_hp(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, object->hp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua_tointeger(lua, 2);
        object->hp(value);
        return 0;
    }
}

int fb::game::life::builtin_mp(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, object->mp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua_tointeger(lua, 2);
        object->mp(value);
        return 0;
    }
}

int fb::game::life::builtin_base_hp(lua_State* lua)
{
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto master = object->based<life::master>();

    lua_pushinteger(lua, master->_hp);
    return 1;
}

int fb::game::life::builtin_base_mp(lua_State* lua)
{
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto master = object->based<life::master>();

    lua_pushinteger(lua, master->_mp);
    return 1;
}

int fb::game::life::builtin_hp_inc(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto value = (uint32_t)lua_tointeger(lua, 2);

    object->hp_up(value);
    return 0;
}

int fb::game::life::builtin_hp_dec(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto value = (uint32_t)lua_tointeger(lua, 2);

    object->hp_down(value);
    return 0;
}

int fb::game::life::builtin_mp_inc(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto value = (uint32_t)lua_tointeger(lua, 2);

    object->mp_up(value);
    return 0;
}

int fb::game::life::builtin_mp_dec(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto value = (uint32_t)lua_tointeger(lua, 2);

    object->mp_down(value);
    return 0;
}

int fb::game::life::builtin_action(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto life = *(fb::game::life**)lua_touserdata(lua, 1);
    auto action = lua_tointeger(lua, 2);
    auto duration = argc < 3 ? fb::game::duration::DURATION_SPELL : lua_tointeger(lua, 3);
    auto sound = argc < 4 ? (uint8_t)0x00 : (uint8_t)lua_tointeger(lua, 4);

    acceptor->send_stream(*life, life->make_action_stream(fb::game::action(action), fb::game::duration(duration), sound), acceptor::scope::PIVOT);
    return 0;
}

int fb::game::life::builtin_spell(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto life = *(fb::game::life**)lua_touserdata(lua, 1);
    auto index = (int)lua_tointeger(lua, 2);

    life->spells[index]->to_lua(lua);
    return 1;
}

int fb::game::life::builtin_damage(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto me = *(fb::game::life**)lua_touserdata(lua, 1);
    auto you = *(fb::game::life**)lua_touserdata(lua, 2);
    auto damage = (uint32_t)lua_tointeger(lua, 3);

    me->hp_down(damage, you, false);
    lua_pushboolean(lua, you->visible());
    return 1;
}