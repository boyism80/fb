#include <fb/model/model.h>
#include <fb/game/builtin/model.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::achievement, "fb.model.achievement")
{"id",                  builtin::model::achievement::builtin_id},
{"look",                builtin::model::achievement::builtin_look},
{"color",               builtin::model::achievement::builtin_color},
{"text",                builtin::model::achievement::builtin_text},
END_LUA_EXTENSION; // clang-format on

int builtin::model::achievement::builtin_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server      = lua->env<fb::game::server>("server");
    auto achievement = lua->touserdata<fb::model::achievement>(1);
    if (achievement == nullptr)
        return 0;

    lua->pushinteger(achievement->id);
    return 1;
}

int builtin::model::achievement::builtin_look(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server      = lua->env<fb::game::server>("server");
    auto achievement = lua->touserdata<fb::model::achievement>(1);
    if (achievement == nullptr)
        return 0;

    lua->pushinteger(achievement->look);
    return 1;
}

int builtin::model::achievement::builtin_color(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server      = lua->env<fb::game::server>("server");
    auto achievement = lua->touserdata<fb::model::achievement>(1);
    if (achievement == nullptr)
        return 0;

    lua->pushinteger(achievement->color);
    return 1;
}

int builtin::model::achievement::builtin_text(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server      = lua->env<fb::game::server>("server");
    auto achievement = lua->touserdata<fb::model::achievement>(1);
    if (achievement == nullptr)
        return 0;

    lua->pushstring(achievement->text);
    return 1;
}