#include <fb/game/server.h>
#include <fb/game/door.h>
#include <fb/game/builtin/door.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(door, "fb.game.door")
{"toggle",              builtin::door::builtin_toggle},
{"lock",                builtin::door::builtin_lock},
{"locked",              builtin::door::builtin_locked},
{"opened",              builtin::door::builtin_opened},
END_LUA_EXTENSION; // clang-format on

int builtin::door::builtin_toggle(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto door   = lua->touserdata<fb::game::door>(1);
    if (door == nullptr)
        return 0;

    auto opened   = std::make_shared<bool>();
    auto map      = server->maps[door->map.model.id];
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        door->toggle();
        *opened = door->opened();

        const auto size = fb::model::size<uint16_t>(door->model.pairs.size(), 1);
        const auto area = fb::model::area<uint16_t>(door->pivot.x,
                                                    door->pivot.y,
                                                    door->pivot.x + size.width,
                                                    door->pivot.y + size.height);
        server->maps.update_map_cache(door->map.model.id, area);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*opened);
        co_return 1;
    };
    return builder.run();
}

int builtin::door::builtin_locked(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto door   = lua->touserdata<fb::game::door>(1);
    if (door == nullptr)
        return 0;

    auto locked   = std::make_shared<bool>();
    auto map      = server->maps[door->map.model.id];
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *locked = door->locked();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*locked);
        co_return 1;
    };
    return builder.run();
}

int builtin::door::builtin_lock(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto door   = lua->touserdata<fb::game::door>(1);
    if (door == nullptr)
        return 0;

    auto value    = lua->toboolean(2);
    auto locked   = std::make_shared<bool>();
    auto map      = server->maps[door->map.model.id];
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        door->lock(value);
        *locked = door->locked();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*locked);
        co_return 1;
    };
    return builder.run();
}

int builtin::door::builtin_opened(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto door   = lua->touserdata<fb::game::door>(1);
    if (door == nullptr)
        return 0;

    auto opened   = std::make_shared<bool>();
    auto map      = server->maps[door->map.model.id];
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *opened = door->opened();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*opened);
        co_return 1;
    };
    return builder.run();
}
