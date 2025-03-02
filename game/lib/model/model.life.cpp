#include <fb/model/model.h>

int fb::model::life::builtin_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto object = lua->touserdata<fb::model::life>(1);
    if (object == nullptr)
        return 0;

    lua->pushinteger(object->hp);
    return 1;
}

int fb::model::life::builtin_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto object = lua->touserdata<fb::model::life>(1);
    if (object == nullptr)
        return 0;

    lua->pushinteger(object->mp);
    return 1;
}