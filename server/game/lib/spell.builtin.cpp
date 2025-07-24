#include <fb/game/spell.h>
#include <fb/game/server.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(spell, "fb.game.spell")
{"model",              spell::builtin::builtin_model},
{"delay",              spell::builtin::builtin_delay},
{"delay2",             spell::builtin::builtin_delay2},
END_LUA_EXTENSION; // clang-format on

int spell::builtin::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto spell  = lua->touserdata<fb::game::spell>(1);
    if (spell == nullptr)
        return 0;

    lua->pushobject(spell->model);
    return 1;
}

int spell::builtin::builtin_delay(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto spell  = lua->touserdata<fb::game::spell>(1);
    if (spell == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(spell->delay());
        return 1;
    }
    else
    {
        spell->delay(lua->tointeger(2));
        return 0;
    }
}

int spell::builtin::builtin_delay2(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto spell  = lua->touserdata<fb::game::spell>(1);
    if (spell == nullptr)
        return 0;

    auto  delay = lua->tointeger(2);
    auto& owner = const_cast<life&>(spell->owner);
    owner.send(fb::protocol::game::response::spell_delay(*spell, delay));
    return 0;
}