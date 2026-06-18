#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/game/builtin/model.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::weapon, "fb.model.weapon")
{"damage_small",        builtin::model::weapon::builtin_damage_small},
{"damage_large",        builtin::model::weapon::builtin_damage_large},
{"sound",               builtin::model::weapon::builtin_sound},
{"type",                builtin::model::weapon::builtin_type},
END_LUA_EXTENSION; // clang-format on

int builtin::model::weapon::builtin_damage_small(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto model = lua->touserdata<fb::model::weapon>(1);

    lua->pushinteger(model->damage_small.min);
    lua->pushinteger(model->damage_small.max);
    return 2;
}

int builtin::model::weapon::builtin_damage_large(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto model = lua->touserdata<fb::model::weapon>(1);

    lua->pushinteger(model->damage_large.min);
    lua->pushinteger(model->damage_large.max);
    return 2;
}

int builtin::model::weapon::builtin_sound(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto model = lua->touserdata<fb::model::weapon>(1);

    lua->pushinteger(model->sound);
    return 1;
}

int builtin::model::weapon::builtin_type(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto model = lua->touserdata<fb::model::weapon>(1);

    lua->pushinteger(model->weapon_type());
    return 1;
}