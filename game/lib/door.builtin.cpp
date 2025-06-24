#include <fb/game/context.h>
#include <fb/game/door.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(door, "fb.game.door")
{"toggle",              door::builtin::builtin_toggle},
{"lock",                door::builtin::builtin_lock},
{"locked",              door::builtin::builtin_locked},
{"opened",              door::builtin::builtin_opened},
END_LUA_EXTENSION; // clang-format on

int door::builtin::builtin_toggle(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto door = lua->touserdata<fb::game::door>(1);

    door->toggle();
    lua->pushboolean(door->opened());

    auto context = lua->env<fb::game::context>("context");
    auto size    = fb::model::size8_t((uint8_t)door->model.pairs.size(), 1);
    for (auto& obj : door->map.nears(door->pivot, OBJECT_TYPE::CHARACTER))
    {
        auto ch = std::static_pointer_cast<character>(obj);
        ch->update_map(door->map, door->pivot, size);
    }
    return 1;
}

int door::builtin::builtin_locked(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto door = lua->touserdata<fb::game::door>(1);

    lua->pushboolean(door->locked());
    return 1;
}

int door::builtin::builtin_lock(lua_State* L)
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

int door::builtin::builtin_opened(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto door = lua->touserdata<fb::game::door>(1);

    lua->pushboolean(door->opened());
    return 1;
}