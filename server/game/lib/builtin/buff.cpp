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
        lua->pushinteger(std::chrono::duration_cast<std::chrono::seconds>(buff->time()).count());
        return 1;
    }
    else
    {
        auto time = std::chrono::seconds(lua->tointeger(2));
        buff->time(time);
        return 0;
    }
}