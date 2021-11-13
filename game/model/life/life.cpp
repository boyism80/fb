#include "model/context/context.game.h"
#include "model/map/map.h"
#include "life.h"

fb::game::life::master::master(const fb::game::life::master::config& config) : 
    object::master(config),
    defensive(config.defensive),
    hp(config.hp),
    mp(config.mp),
    experience(config.exp)
{ }

fb::game::life::master::~master()
{ }

int fb::game::life::master::builtin_hp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto object = thread->touserdata<fb::game::life::master>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushinteger(object->hp);
    return 1;
}

int fb::game::life::master::builtin_mp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto object = thread->touserdata<fb::game::life::master>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushinteger(object->mp);
    return 1;
}

fb::game::life::life(fb::game::context& context, const master* master, const fb::game::life::config& config) : 
    object(context, master, config),
    _hp(config.hp),
    _mp(config.mp),
    spells(*this)
{ }

fb::game::life::~life()
{ }

uint32_t fb::game::life::calculate_damage(uint32_t value, const fb::game::life& life) const
{
    auto rate = life.direction() == this->direction() ? 2 : 1;
    auto n = (100 - life.defensive_physical()) / 10;
    auto defensive_percent = -125 + (n * (2*14.75f - (n-1)/2.0f))/2.0f;
    auto damage = value - uint32_t(defensive_percent * (value/100.0f));

    return damage * rate;
}

uint32_t fb::game::life::hp_up(uint32_t value, fb::game::object* from)
{
    auto listener = this->get_listener<fb::game::life>();

    value = std::min(value, this->base_hp() - this->_hp);
    this->hp(this->_hp + value);
    listener->on_heal_hp(*this, value, from);

    return value;
}

uint32_t fb::game::life::hp_down(uint32_t value, fb::game::object* from, bool critical)
{
    auto listener = this->get_listener<fb::game::life>();

    value = std::min(value, this->_hp);
    this->hp(this->_hp - value);

    this->handle_damaged(from, value, critical);
    if(this->_hp == 0)
    {
        if(from != nullptr)
            from->handle_kill(*this);
        this->handle_die(from);
    }

    return value;
}

uint32_t fb::game::life::mp_up(uint32_t value, fb::game::object* from)
{
    auto listener = this->get_listener<fb::game::life>();

    value = std::min(value, this->base_mp() - this->_mp);
    this->mp(this->_mp + value);
    listener->on_heal_mp(*this, value, from);

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

    if(this->alive() == false)
        return;

    auto front = this->forward(object::types::UNKNOWN);
    this->handle_attack(front);

    if(front == nullptr || front->is(fb::game::object::types::LIFE) == false)
        return;

    auto you = static_cast<fb::game::life*>(front);
    if(you == nullptr)
        return;

    auto miss = this->handle_calculate_miss(*you);
    if(miss)
        return;

    auto critical = this->handle_calculate_critical(*you);
    auto damage = this->handle_calculate_damage(critical);

    this->handle_hit(*you, damage, critical);
}

uint32_t fb::game::life::hp() const
{
    return this->_hp;
}

void fb::game::life::hp(uint32_t value)
{
    auto before = this->_hp;
    this->_hp = value;

    this->handle_update();

    auto listener = this->get_listener<fb::game::life>();
    listener->on_hp(*this, before, this->_hp);
}

uint32_t fb::game::life::mp() const
{
    return this->_mp;
}

void fb::game::life::mp(uint32_t value)
{
    auto before = this->_mp;
    this->_mp = value;

    this->handle_update();

    auto listener = this->get_listener<fb::game::life>();
    listener->on_mp(*this, before, this->_hp);
}

uint32_t fb::game::life::base_hp() const
{
    return static_cast<const master*>(this->_master)->hp;
}

uint32_t fb::game::life::base_mp() const
{
    return static_cast<const master*>(this->_master)->mp;
}

uint32_t fb::game::life::experience() const
{
    return static_cast<const master*>(this->_master)->experience;
}

uint32_t fb::game::life::defensive_physical() const
{
    return static_cast<const master*>(this->_master)->defensive.physical;
}

uint32_t fb::game::life::defensive_magical() const
{
    return static_cast<const master*>(this->_master)->defensive.magical;
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

    auto listener = this->get_listener<fb::game::life>();
    listener->on_hide(*this);
}

bool fb::game::life::active(fb::game::spell& spell, const std::string& message)
{
    auto& thread = fb::game::lua::get();
    thread.from(spell.cast().c_str())
        .func("on_cast");

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
    auto& thread = fb::game::lua::get();
    thread.from(spell.cast().c_str())
        .func("on_cast");

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
        .pushobject(&to)
        .pushobject(spell)
        .resume(3);
    return true;
}

bool fb::game::life::active(fb::game::spell& spell)
{
    auto& thread = fb::game::lua::get();
    thread.from(spell.cast().c_str())
        .func("on_cast");

    if(spell.type() != fb::game::spell::types::NORMAL)
        return false;

    thread.pushobject(this)
        .pushobject(spell)
        .resume(2);
    return true;
}

bool fb::game::life::handle_calculate_critical(fb::game::life& you) const
{
#if defined DEBUG | defined _DEBUG
    return true;
#else
    return std::rand() % 100 < 20;
#endif
}

bool fb::game::life::handle_calculate_miss(fb::game::life& you) const
{
#if defined DEBUG | defined _DEBUG
    return false;
#else
    return std::rand() % 3 == 0;
#endif
}

void fb::game::life::handle_attack(fb::game::object* you)
{
    auto listener = this->get_listener<fb::game::life>();
    listener->on_attack(*this, you);
}

void fb::game::life::handle_hit(fb::game::life& you, uint32_t damage, bool critical)
{
    auto listener = this->get_listener<fb::game::life>();
    listener->on_hit(*this, you, damage, critical);
}

void fb::game::life::handle_damaged(fb::game::object* from, uint32_t damage, bool critical)
{
    auto listener = this->get_listener<fb::game::life>();
    listener->on_damaged(*this, from, damage, critical);
}

void fb::game::life::handle_die(fb::game::object* from)
{
    auto listener = this->get_listener<fb::game::life>();
    listener->on_die(*this, from);
}

void fb::game::life::handle_kill(fb::game::life& you)
{
    auto listener = this->get_listener<fb::game::life>();
    listener->on_kill(*this, you);
}


int fb::game::life::builtin_hp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || context->exists(*object) == false)
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
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || context->exists(*object) == false)
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
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto master = object->based<fb::game::life>();

    thread->pushinteger(master->hp);
    return 1;
}

int fb::game::life::builtin_base_mp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto master = object->based<fb::game::life>();

    thread->pushinteger(master->mp);
    return 1;
}

int fb::game::life::builtin_hp_inc(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto value = (uint32_t)thread->tointeger(2);

    object->hp_up(value);
    return 0;
}

int fb::game::life::builtin_hp_dec(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto value = (uint32_t)thread->tointeger(2);

    object->hp_down(value);
    return 0;
}

int fb::game::life::builtin_mp_inc(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto value = (uint32_t)thread->tointeger(2);

    object->mp_up(value);
    return 0;
}

int fb::game::life::builtin_mp_dec(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::life>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto value = (uint32_t)thread->tointeger(2);

    object->mp_down(value);
    return 0;
}

int fb::game::life::builtin_action(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto life = thread->touserdata<fb::game::life>(1);
    if(life == nullptr || context->exists(*life) == false)
        return 0;
    
    auto action = thread->tointeger(2);
    auto duration = argc < 3 ? static_cast<int>(fb::game::duration::DURATION_SPELL) : thread->tointeger(3);
    auto sound = argc < 4 ? (uint8_t)0x00 : (uint8_t)thread->tointeger(4);

    context->send(*life, fb::protocol::game::response::life::action(*life, fb::game::action(action), fb::game::duration(duration), sound), context::scope::PIVOT);
    return 0;
}

int fb::game::life::builtin_spell(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto life = thread->touserdata<fb::game::life>(1);
    if(life == nullptr || context->exists(*life) == false)
        return 0;
    
    auto index = (int)thread->tointeger(2);

    life->spells[index]->to_lua(lua);
    return 1;
}

int fb::game::life::builtin_damage(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto me = thread->touserdata<fb::game::life>(1);
    if(me == nullptr || context->exists(*me) == false)
        return 0;

    auto you = thread->touserdata<fb::game::life>(2);
    if(you == nullptr || context->exists(*you) == false)
        return 0;

    auto damage = (uint32_t)thread->tointeger(3);

    me->hp_down(damage, you, false);
    thread->pushboolean(you->visible());
    return 1;
}

int fb::game::life::builtin_cast(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto me = thread->touserdata<fb::game::life>(1);
    if(me == nullptr || context->exists(*me) == false)
        return 0;

    auto you = thread->touserdata<fb::game::life>(2);
    if(you == nullptr || context->exists(*you) == false)
        you = nullptr;

    auto name = thread->tostring(3);
    auto spell = fb::game::table::spells.name2spell(name);
    if(spell == nullptr)
        return 0;

    auto& x = lua::get();
    x.from(spell->cast().c_str())
     .func("on_cast")
     .pushobject(me);

    if(you != nullptr)
        x.pushobject(you);
    else
        x.pushnil();

    x.pushobject(spell)
     .resume(3);

    return 0;
}