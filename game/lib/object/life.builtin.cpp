#include <fb/game/context.h>
#include <fb/game/map.h>
#include <fb/game/life.h>

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
    
    auto& model = object->based<fb::model::life>();

    thread->pushinteger(model.hp);
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
    
    auto& model = object->based<fb::model::life>();

    thread->pushinteger(model.mp);
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
    auto duration = argc < 3 ? static_cast<int>(DURATION::SPELL) : thread->tointeger(3);
    auto sound = argc < 4 ? (uint8_t)0x00 : (uint8_t)thread->tointeger(4);

    context->send(*life, fb::protocol::game::response::life::action(*life, ACTION(action), DURATION(duration), sound), context::scope::PIVOT);
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

    thread->pushobject(life->spells[index]);
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
    auto spell = context->model.spell.name2spell(name);
    if(spell == nullptr)
        return 0;

    auto x = lua::get();
    if(x == nullptr)
        return 0;

    x->from(spell->cast.c_str())
     .func("on_cast")
     .pushobject(me);

    if(you != nullptr)
        x->pushobject(you);
    else
        x->pushnil();

    x->pushobject(spell)
     .resume(3);

    return 0;
}