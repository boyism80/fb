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
    auto argc   = lua->argc();
    auto door   = lua->touserdata<fb::game::door>(1);

    door->toggle();
    lua->pushboolean(door->opened());

    const auto size = fb::model::size<uint16_t>(door->model.pairs.size(), 1);
    const auto area = fb::model::area<uint16_t>(door->pivot.x,
                                                door->pivot.y,
                                                door->pivot.x + size.width,
                                                door->pivot.y + size.height);
    server->update_map_cache(door->map.model.id, area);
    return 1;
}

int builtin::door::builtin_locked(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto door = lua->touserdata<fb::game::door>(1);

    lua->pushboolean(door->locked());
    return 1;
}

int builtin::door::builtin_lock(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc  = lua->argc();
    auto door  = lua->touserdata<fb::game::door>(1);
    auto value = lua->toboolean(2);

    door->lock(value);
    lua->pushboolean(door->locked());
    return 1;
}

int builtin::door::builtin_opened(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto door = lua->touserdata<fb::game::door>(1);

    lua->pushboolean(door->opened());
    return 1;
}