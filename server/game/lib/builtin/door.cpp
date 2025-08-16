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

    auto argc = lua->argc();
    auto door = lua->touserdata<fb::game::door>(1);

    door->toggle();
    lua->pushboolean(door->opened());

    auto server = lua->env<fb::game::server>("server");
    auto size   = fb::model::size8_t((uint8_t)door->model.pairs.size(), 1);
    for (auto& obj : door->map.nears(door->pivot, OBJECT_TYPE::CHARACTER))
    {
        auto ch = std::static_pointer_cast<character>(obj);
        ch->update_map(door->map, door->pivot, size);
    }
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