#include <fb/game/model.h>

int fb::model::life::builtin_hp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto object = thread->touserdata<fb::model::life>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushinteger(object->hp);
    return 1;
}

int fb::model::life::builtin_mp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto object = thread->touserdata<fb::model::life>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushinteger(object->mp);
    return 1;
}