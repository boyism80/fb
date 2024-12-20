#include <trace.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::trace, "fb.game.trace")
{"model",              	fb::game::trace::builtin_model},
{"text",               	fb::game::trace::builtin_text},
END_LUA_EXTENSION; // clang-format on

int fb::game::trace::builtin_model(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx   = thread->env<fb::game::context>("context");
    auto trace = thread->touserdata<fb::game::trace>(1);
    if (trace == nullptr)
        return 0;

    thread->pushobject(trace->model);
    return 1;
}

int fb::game::trace::builtin_text(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx   = thread->env<fb::game::context>("context");
    auto trace = thread->touserdata<fb::game::trace>(1);
    if (trace == nullptr)
        return 0;

    thread->pushstring(trace->text.value_or(trace->model.text));
    return 1;
}