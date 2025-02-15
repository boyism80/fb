#include <fb/game/mob.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::mob, "fb.game.mob")
{"__eq",                fb::game::object::builtin_eq},
{"target",              fb::game::mob::builtin_target},
{"oblivion",            fb::game::mob::builtin_oblivion},
END_LUA_EXTENSION; // clang-format on

int mob::builtin_target(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto mob  = thread->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    auto target = (fb::game::life*)nullptr;
    if (argc > 1 && lua_type(lua, 2) != LUA_TNIL)
        target = thread->touserdata<fb::game::life>(2);

    auto n = (argc == 1 ? 1 : 0);
    return ctx->builtin(*mob, thread, n, [=]() -> async::task<void> {
        if (argc == 1)
        {
            if (mob->_target == nullptr)
                thread->pushnil();
            else
                thread->pushobject(mob->_target);
        }
        else
        {
            mob->target(target);
        }

        co_return;
    });
}

int mob::builtin_oblivion(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto mob  = thread->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    auto oblivion = (fb::game::life*)nullptr;
    if (argc > 1 && lua_type(lua, 2) != LUA_TNIL)
        oblivion = thread->touserdata<fb::game::life>(2);

    auto n = (argc == 1 ? 1 : 0);
    return ctx->builtin(*mob, thread, n, [=]() -> async::task<void> {
        if (argc == 1)
        {
            if (mob->_oblivion == nullptr)
                thread->pushnil();
            else
                thread->pushobject(mob->_oblivion);
        }
        else
        {
            mob->oblivion(oblivion);
        }
        co_return;
    });
}