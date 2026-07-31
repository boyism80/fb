#include <fb/bot/integration/lua_integration_model.h>
#include <fb/model/model.h>

using table = fb::model::table;

namespace {

using CARDINAL_DIRECTION = fb::model::enum_value::CARDINAL_DIRECTION;

int builtin_name2map(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto name = lua->tostring(1);
    auto map  = table::map->name2map(name);

    if (map == nullptr)
        lua->pushnil();
    else
        lua->pushobject(map);

    return 1;
}

int builtin_id2map(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto id  = static_cast<uint32_t>(lua->tointeger(1));
    auto map = const_cast<fb::model::map*>(table::map->find(id));

    if (map == nullptr)
        lua->pushnil();
    else
        lua->pushobject(map);

    return 1;
}

int map_builtin_id(lua_State* L)
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

int map_builtin_name(lua_State* L)
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

int map_builtin_root(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    if (table::map->contains(map->root) == false)
    {
        lua->pushnil();
        return 1;
    }

    auto  map_table = table::map;
    auto& root      = map_table[map->root];
    lua->pushobject(root);
    return 1;
}

int map_builtin_revive(lua_State* L)
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
    for (auto& [direction, target_id] : map->revive)
    {
        lua->pushinteger(static_cast<lua_Integer>(direction));
        lua->pushobject(table::map[target_id]);
        lua_settable(L, -3);
    }

    return 1;
}

void register_cardinal_direction(lua_State* L)
{
    lua_newtable(L);
    lua_pushinteger(L, static_cast<lua_Integer>(CARDINAL_DIRECTION::EAST));
    lua_setfield(L, -2, "EAST");
    lua_pushinteger(L, static_cast<lua_Integer>(CARDINAL_DIRECTION::WEST));
    lua_setfield(L, -2, "WEST");
    lua_pushinteger(L, static_cast<lua_Integer>(CARDINAL_DIRECTION::SOUTH));
    lua_setfield(L, -2, "SOUTH");
    lua_pushinteger(L, static_cast<lua_Integer>(CARDINAL_DIRECTION::NORTH));
    lua_setfield(L, -2, "NORTH");
    lua_setglobal(L, "CARDINAL_DIRECTION");
}

} // namespace

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::map, "fb.model.map")
{"id",     map_builtin_id},
{"name",   map_builtin_name},
{"root",   map_builtin_root},
{"revive", map_builtin_revive},
END_LUA_EXTENSION; // clang-format on

namespace fb::bot::integration {

void register_model_globals(fb::lua::root& lua)
{
    lua.build<fb::model::map, fb::lua::luable>();
    lua.build("name2map", builtin_name2map);
    lua.build("id2map", builtin_id2map);
    register_cardinal_direction((lua_State*)lua);
}

} // namespace fb::bot::integration
