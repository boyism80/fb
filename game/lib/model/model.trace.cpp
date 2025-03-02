#include <fb/model/model.h>

int fb::model::trace::builtin_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto trace   = lua->touserdata<fb::model::trace>(1);
    if (trace == nullptr)
        return 0;

    lua->pushinteger(trace->id);
    return 1;
}

int fb::model::trace::builtin_look(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto trace   = lua->touserdata<fb::model::trace>(1);
    if (trace == nullptr)
        return 0;

    lua->pushinteger(trace->look);
    return 1;
}

int fb::model::trace::builtin_color(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto trace   = lua->touserdata<fb::model::trace>(1);
    if (trace == nullptr)
        return 0;

    lua->pushinteger(trace->color);
    return 1;
}

int fb::model::trace::builtin_text(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto trace   = lua->touserdata<fb::model::trace>(1);
    if (trace == nullptr)
        return 0;

    lua->pushstring(trace->text);
    return 1;
}