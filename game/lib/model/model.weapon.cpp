#include <fb/game/context.h>
#include <fb/game/item.h>
#include <fb/model/model.h>

int fb::model::weapon::builtin_damage_small(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto model   = lua->touserdata<fb::model::weapon>(1);

    lua->pushinteger(model->damage_small.min);
    lua->pushinteger(model->damage_small.max);
    return 2;
}

int fb::model::weapon::builtin_damage_large(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto model   = lua->touserdata<fb::model::weapon>(1);

    lua->pushinteger(model->damage_large.min);
    lua->pushinteger(model->damage_large.max);
    return 2;
}

int fb::model::weapon::builtin_sound(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto model   = lua->touserdata<fb::model::weapon>(1);

    lua->pushinteger(model->sound);
    return 1;
}

int fb::model::weapon::builtin_type(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto model   = lua->touserdata<fb::model::weapon>(1);

    lua->pushinteger(model->weapon_type());
    return 1;
}