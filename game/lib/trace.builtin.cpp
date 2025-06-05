#include <fb/game/trace.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(trace, "fb.game.trace")
{"model",              	trace::builtin::builtin_model},
{"text",               	trace::builtin::builtin_text},
END_LUA_EXTENSION; // clang-format on

int trace::builtin::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx   = lua->env<fb::game::context>("context");
    auto trace = lua->touserdata<fb::game::trace>(1);
    if (trace == nullptr)
        return 0;

    lua->pushobject(trace->model);
    return 1;
}

int trace::builtin::builtin_text(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx   = lua->env<fb::game::context>("context");
    auto trace = lua->touserdata<fb::game::trace>(1);
    if (trace == nullptr)
        return 0;

    lua->pushstring(trace->text.value_or(trace->model.text));
    return 1;
}