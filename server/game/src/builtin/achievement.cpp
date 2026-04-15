#include <fb/game/achievement.h>
#include <fb/game/builtin/achievement.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(achievement, "fb.game.achievement")
{"id",                  builtin::achievement::builtin_id},
{"text",                builtin::achievement::builtin_text},
{"icon",                builtin::achievement::builtin_icon},
{"color",               builtin::achievement::builtin_color},
END_LUA_EXTENSION; // clang-format on

int builtin::achievement::builtin_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ach = lua->touserdata<fb::game::achievement>(1);
    if (ach == nullptr)
        return 0;

    lua->pushinteger(ach->id);
    return 1;
}

int builtin::achievement::builtin_text(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ach = lua->touserdata<fb::game::achievement>(1);
    if (ach == nullptr)
        return 0;

    lua->pushstring(ach->text);
    return 1;
}

int builtin::achievement::builtin_icon(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ach = lua->touserdata<fb::game::achievement>(1);
    if (ach == nullptr)
        return 0;

    lua->pushinteger(ach->icon);
    return 1;
}

int builtin::achievement::builtin_color(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ach = lua->touserdata<fb::game::achievement>(1);
    if (ach == nullptr)
        return 0;

    lua->pushinteger(ach->color);
    return 1;
}