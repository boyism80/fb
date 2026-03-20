#include <fb/game/spell.h>
#include <fb/game/server.h>
#include <fb/game/builtin/spell.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(buff, "fb.game.buff")
{"model",              builtin::buff::builtin_model},
{"time",               builtin::buff::builtin_time},
END_LUA_EXTENSION; // clang-format on

int builtin::buff::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto buff = lua->touserdata<fb::game::buff>(1);
    if (buff == nullptr)
        return 0;

    lua->pushobject(buff->model);
    return 1;
}

int builtin::buff::builtin_time(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto buff = lua->touserdata<fb::game::buff>(1);
    if (buff == nullptr)
        return 0;

    if (argc == 1)
    {
        auto remaining_ms = buff->remaining().total_milliseconds();
        if (remaining_ms < 0)
            remaining_ms = 0;
        lua->pushinteger(remaining_ms / 1000);
        return 1;
    }
    else
    {
        auto seconds = lua->tointeger(2);
        buff->remaining(fb::model::timespan{std::chrono::seconds(seconds)});
        return 0;
    }
}