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

    if (argc == 1)
    {
        if (mob->thread() == ctx->threads.current())
        {
            if (mob->_target == nullptr)
                thread->pushnil();
            else
                thread->pushobject(mob->_target);
            return 1;
        }
        else
        {
            ctx->threads.enqueue(*mob, [=](auto&) -> async::task<void> {
                if (mob->_target == nullptr)
                    thread->pushnil();
                else
                    thread->pushobject(mob->_target);
                thread->resume(1);
                co_return;
            });
            return thread->yield(1);
        }
    }
    else
    {
        auto target = lua_type(lua, 2) != LUA_TNIL ? thread->touserdata<fb::game::life>(2) : nullptr;
        if (mob->thread() == ctx->threads.current())
        {
            mob->target(target);
            return 0;
        }
        else
        {
            ctx->threads.enqueue(*mob, [=](auto&) -> async::task<void> {
                mob->target(target);
                thread->resume(0);
                co_return;
            });
            return thread->yield(0);
        }
    }
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

    if (argc == 1)
    {
        if (mob->thread() == ctx->threads.current())
        {
            if (mob->_oblivion == nullptr)
                thread->pushnil();
            else
                thread->pushobject(mob->_oblivion);
            return 1;
        }
        else
        {
            ctx->threads.enqueue(*mob, [=](auto&) -> async::task<void> {
                if (mob->_oblivion == nullptr)
                    thread->pushnil();
                else
                    thread->pushobject(mob->_oblivion);
                thread->resume(1);
                co_return;
            });
            return thread->yield(1);
        }
    }
    else
    {
        auto oblivion = lua_type(lua, 2) != LUA_TNIL ? thread->touserdata<fb::game::life>(2) : nullptr;
        if (mob->thread() == ctx->threads.current())
        {
            mob->oblivion(oblivion);
            return 0;
        }
        else
        {
            ctx->threads.enqueue(*mob, [=](auto&) -> async::task<void> {
                mob->oblivion(oblivion);
                thread->resume(0);
                co_return;
            });
            return thread->yield(0);
        }
    }
}