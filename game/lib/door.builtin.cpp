#include <context.h>
#include <door.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::door, "fb.game.door")
{"toggle",              fb::game::door::builtin_toggle},
{"lock",                fb::game::door::builtin_lock},
{"locked",              fb::game::door::builtin_locked},
{"opened",              fb::game::door::builtin_opened},
END_LUA_EXTENSION; // clang-format on

int fb::game::door::builtin_toggle(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto door = thread->touserdata<fb::game::door>(1);

    door->toggle();
    lua_pushboolean(lua, door->opened());

    auto context = thread->env<fb::game::context>("context");
    auto size    = size8_t((uint8_t)door->model.pairs.size(), 1);
    async::awaitable_get(
        context->send(fb::protocol::game::response::map::update(door->map, door->pivot, size), door->map));
    return 1;
}

int fb::game::door::builtin_locked(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto door = thread->touserdata<fb::game::door>(1);

    lua_pushboolean(lua, door->locked());
    return 1;
}

int fb::game::door::builtin_lock(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc  = thread->argc();
    auto door  = thread->touserdata<fb::game::door>(1);
    auto value = thread->toboolean(2);

    door->lock(value);
    lua_pushboolean(lua, door->locked());
    return 1;
}

int fb::game::door::builtin_opened(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto door = thread->touserdata<fb::game::door>(1);

    lua_pushboolean(lua, door->opened());
    return 1;
}