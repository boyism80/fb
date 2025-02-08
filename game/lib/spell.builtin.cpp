#include <fb/game/spell.h>
#include <fb/game/context.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::spell, "fb.game.spell")
{"model",              fb::game::spell::builtin_model},
{"delay",              fb::game::spell::builtin_delay},
END_LUA_EXTENSION; // clang-format on

int fb::game::spell::builtin_model(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx   = thread->env<fb::game::context>("context");
    auto argc  = thread->argc();
    auto spell = thread->touserdata<fb::game::spell>(1);
    if (spell == nullptr)
        return 0;

    thread->pushobject(spell->model);
    return 1;
}

int fb::game::spell::builtin_delay(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx   = thread->env<fb::game::context>("context");
    auto argc  = thread->argc();
    auto spell = thread->touserdata<fb::game::spell>(1);
    if (spell == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(spell->delay());
        return 1;
    }
    else
    {
        spell->delay(thread->tointeger(2));
        return 0;
    }
}