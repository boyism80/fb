#include <fb/game/spell.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(buff, "fb.game.buff")
{"model",              buff::builtin::builtin_model},
{"time",               buff::builtin::builtin_time},
END_LUA_EXTENSION; // clang-format on

int buff::builtin::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto buff = lua->touserdata<fb::game::buff>(1);
    if (buff == nullptr)
        return 0;

    lua->pushobject(buff->model);
    return 1;
}

int buff::builtin::builtin_time(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
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