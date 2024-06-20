#include <fb/game/session.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/game/model.h>

int fb::model::mob::builtin_speed(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto mob = thread->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    lua_pushinteger(lua, mob->speed.count());
    return 1;
}

fb::model::mob* fb::model::__mob::name2mob(const std::string& name) const
{
    auto i = std::find_if(this->begin(), this->end(), 
        [&name](auto& pair)
        {
            return pair.second.name == name;
        });

    return i != this->end() ? i->second.get() : nullptr;
}