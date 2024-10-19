#include <spell.h>

int fb::model::spell::builtin_type(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto spell = thread->touserdata<fb::model::spell>(1);
    if(spell == nullptr)
        return 0;

    thread->pushinteger(spell->type);
    return 1;
}

int fb::model::spell::builtin_name(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto spell = thread->touserdata<fb::model::spell>(1);
    if(spell == nullptr)
        return 0;

    thread->pushstring(spell->name);
    return 1;
}

int fb::model::spell::builtin_message(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto spell = thread->touserdata<fb::model::spell>(1);
    if(spell == nullptr)
        return 0;

    thread->pushstring(spell->message);
    return 1;
}