#include <fb/model/model.h>
#include <fb/game/builtin/model.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::life, "fb.model.life")
{"hp",                  builtin::model::life::builtin_hp},
{"mp",                  builtin::model::life::builtin_mp},
END_LUA_EXTENSION; // clang-format on

int builtin::model::life::builtin_hp(lua_State* L)
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

int builtin::model::life::builtin_mp(lua_State* L)
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