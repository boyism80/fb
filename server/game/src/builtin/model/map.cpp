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
{"clone",               builtin::model::map::builtin_clone},
{"instance",            builtin::model::map::builtin_instance},
{"instance_rule",       builtin::model::map::builtin_instance_rule},
{"instance_capacity",   builtin::model::map::builtin_instance_capacity},
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
    if (table::map->contains(params.id) == false)
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

int builtin::model::map::builtin_clone(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::map>(1);
    if (model == nullptr)
        return 0;

    auto& server = static_cast<fb::game::server&>(lua->executor);
    if (server.maps.contains(model->id) == false)
        return 0;

    auto source   = server.maps[model->id];
    auto holder   = std::make_shared<std::shared_ptr<fb::game::map>>();
    auto weak     = source->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=, server = &server]() -> async::task<void> {
        *holder = server->maps.clone(source);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::model::map::builtin_instance(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::map>(1);
    if (model == nullptr)
        return 0;

    if (lua->argc() < 2 || lua->is_number(2) == false)
        return 0;

    auto& server = static_cast<fb::game::server&>(lua->executor);
    if (server.maps.contains(model->id) == false)
        return 0;

    auto slot     = static_cast<uint32_t>(lua->tointeger(2));
    auto source   = server.maps[model->id];
    auto holder   = std::make_shared<std::shared_ptr<fb::game::map>>();
    auto weak     = source->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=, server = &server]() -> async::task<void> {
        *holder = server->maps.ensure_instance(source, slot);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*holder);
        co_return 1;
    };
    return builder.run();
}
int builtin::model::map::builtin_instance_rule(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::map>(1);
    if (model == nullptr)
        return 0;

    lua->pushinteger(static_cast<lua_Integer>(model->instance_rule));
    return 1;
}

int builtin::model::map::builtin_instance_capacity(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::map>(1);
    if (model == nullptr)
        return 0;

    if (model->instance_capacity.has_value() == false)
        lua->pushnil();
    else
        lua->pushinteger(model->instance_capacity.value());
    return 1;
}
