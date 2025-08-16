#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/game/builtin/model.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::spell, "fb.model.spell")
{"type",                builtin::model::spell::builtin_type},
{"name",                builtin::model::spell::builtin_name},
{"message",             builtin::model::spell::builtin_message},
END_LUA_EXTENSION; // clang-format on

int builtin::model::spell::builtin_type(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto spell = lua->touserdata<fb::model::spell>(1);
    if (spell == nullptr)
        return 0;

    lua->pushinteger(spell->type);
    return 1;
}

int builtin::model::spell::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto spell = lua->touserdata<fb::model::spell>(1);
    if (spell == nullptr)
        return 0;

    lua->pushstring(spell->name);
    return 1;
}

int builtin::model::spell::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto spell = lua->touserdata<fb::model::spell>(1);
    if (spell == nullptr)
        return 0;

    lua->pushstring(spell->message);
    return 1;
}