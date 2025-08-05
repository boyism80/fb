#include <fb/game/quest.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(quest, "fb.game.quest")
{"model",               quest::builtin_model},
{"step",                quest::builtin_step},
{"progress",            quest::builtin_progress},
{"inc_progress",        quest::builtin_inc_progress},
{"inc_step",            quest::builtin_inc_step},
{"complete",            quest::builtin_complete},
{"completed",           quest::builtin_completed},
END_LUA_EXTENSION; // clang-format on

int quest::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto quest = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    lua->pushinteger(quest->id);
    return 1;
}

int quest::builtin_step(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto quest = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    lua->pushinteger(quest->step());
    return 1;
}

int quest::builtin_progress(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto quest = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    lua->pushinteger(quest->progress());
    return 1;
}

int quest::builtin_inc_progress(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto quest = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    if (value < 0)
        return 0;

    lua->pushboolean(quest->inc_progress(value));
    return 1;
}

int quest::builtin_inc_step(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto quest = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    if (value < 0)
        return 0;

    lua->pushboolean(quest->inc_step(value));
    return 1;
}

int quest::builtin_complete(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto quest = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    lua->pushboolean(quest->complete());
    return 1;
}

int quest::builtin_completed(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto quest = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    lua->pushboolean(quest->completed());
    return 1;
}