#include <fb/game/character.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/model/model.h>

int fb::model::mob::builtin_speed(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto mob = thread->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    thread->pushinteger(mob->speed.total_milliseconds());
    return 1;
}

int fb::model::mob::builtin_size(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto mob = thread->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    thread->pushinteger(static_cast<uint8_t>(mob->size));
    return 1;
}

int fb::model::mob::builtin_damage(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto mob = thread->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    thread->pushinteger(mob->damage.min);
    thread->pushinteger(mob->damage.max);
    return 2;
}

fb::model::mob* fb::model::__mob::name2mob(const std::string& name) const
{
    for (auto& [k, v] : *this)
    {
        if (v.name == name)
            return &v;
    }
    return nullptr;
}