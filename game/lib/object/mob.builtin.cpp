#include <fb/game/session.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/core/thread.h>

int fb::model::mob::builtin_speed(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto mob = thread->touserdata<fb::model::mob>(1);
    if(mob == nullptr)
        return 0;

    lua_pushinteger(lua, mob->speed.count());
    return 1;
}