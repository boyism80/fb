#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/game/builtin/model.h>

using namespace fb::game;
using table = fb::model::table;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::map, "fb.model.map")
{"id",                  builtin::model::map::builtin_id},
{"name",                builtin::model::map::builtin_name},
{"root",                builtin::model::map::builtin_root},
{"cardinal",            builtin::model::map::builtin_cardinal},
{"revive",              builtin::model::map::builtin_revive},
{"option",              builtin::model::map::builtin_option},
END_LUA_EXTENSION; // clang-format on

int builtin::model::map::builtin_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    lua->pushinteger(map->id);
    return 1;
}

int builtin::model::map::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    lua->pushstring(map->name);
    return 1;
}

int builtin::model::map::builtin_root(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto map = lua->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    if (table::map.contains(map->root) == false)
    {
        lua->pushnil();
        return 1;
    }

    auto& root = table::map[map->root];
    lua->pushobject(root);
    return 1;
}

int builtin::model::map::builtin_cardinal(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    if (argc != 2)
    {
        lua->pushnil();
        return 1;
    }

    auto map = lua->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    auto direction = static_cast<CARDINAL_DIRECTION>(lua->tointeger(2));
    if (map->cardinal.contains(direction) == false)
    {
        lua->pushnil();
        return 1;
    }

    auto& dsl = map->cardinal.at(direction);
    if (dsl.header != DSL::map)
    {
        lua->pushnil();
        return 1;
    }

    auto params = fb::model::dsl::map(dsl.params);
    if (table::map.contains(params.id) == false)
    {
        lua->pushnil();
        return 1;
    }

    lua->new_table();
    {
        lua->pushinteger(1);
        lua->pushobject(table::map[params.id]);
        lua_settable(L, -3);

        lua->pushinteger(2);
        lua->pushinteger(params.x);
        lua_settable(L, -3);

        lua->pushinteger(3);
        lua->pushinteger(params.y);
        lua_settable(L, -3);

        lua->pushinteger(4);
        lua->pushinteger(params.right);
        lua_settable(L, -3);

        lua->pushinteger(5);
        lua->pushinteger(params.bottom);
        lua_settable(L, -3);
    }
    return 1;
}

int builtin::model::map::builtin_revive(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto map = lua->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    if (map->revive.size() == 0)
    {
        lua->pushnil();
        return 1;
    }

    lua->new_table();
    for (auto& [direction, x] : map->revive)
    {
        lua->pushinteger(direction);
        lua->pushobject(table::map[x]);
        lua_settable(L, -3);
    }

    return 1;
}

int builtin::model::map::builtin_option(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto map = lua->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    lua->pushinteger(map->option);
    return 1;
}