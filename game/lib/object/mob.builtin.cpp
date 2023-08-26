#include <fb/game/session.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/core/thread.h>

int fb::game::mob::model::builtin_speed(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto mob = thread->touserdata<fb::game::mob::model>(1);
    if(mob == nullptr)
        return 0;

    lua_pushinteger(lua, mob->speed.count());
    return 1;
}