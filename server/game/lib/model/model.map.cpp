#include <fb/model/model.h>
#include <fb/game/server.h>

int fb::model::map::builtin_id(lua_State* L)
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

int fb::model::map::builtin_name(lua_State* L)
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

int fb::model::map::builtin_root(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto map    = lua->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    if (server->model.map.contains(map->root) == false)
    {
        lua->pushnil();
        return 1;
    }

    auto& root = server->model.map[map->root];
    lua->pushobject(root);
    return 1;
}

int fb::model::map::builtin_cardinal(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
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

    auto params = dsl::map(dsl.params);
    if (server->model.map.contains(params.id) == false)
    {
        lua->pushnil();
        return 1;
    }

    lua->new_table();
    {
        lua->pushinteger(1);
        lua->pushobject(server->model.map[params.id]);
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

int fb::model::map::builtin_revive(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto map    = lua->touserdata<fb::model::map>(1);
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
        lua->pushobject(server->model.map[x]);
        lua_settable(L, -3);
    }

    return 1;
}

int fb::model::map::builtin_option(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto map    = lua->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    lua->pushinteger(map->option);
    return 1;
}

fb::model::map* fb::model::__map::name2map(const std::string& name) const
{
    for (auto& [k, v] : *this)
    {
        if (v.name == name)
            return &v;
    }

    return nullptr;
}