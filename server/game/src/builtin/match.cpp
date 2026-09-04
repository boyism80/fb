#include <fb/game/server.h>
#include <fb/game/match.h>
#include <fb/game/builtin/match.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(match, "fb.game.match")
{"id",            builtin::match::builtin_id},
{"type",          builtin::match::builtin_type},
{"state",         builtin::match::builtin_state},
{"wait",          builtin::match::builtin_wait},
{"finish",        builtin::match::builtin_finish},
{"members",       builtin::match::builtin_members},
{"member_count",  builtin::match::builtin_member_count},
{"expected",      builtin::match::builtin_expected},
{"map",           builtin::match::builtin_map},
END_LUA_EXTENSION; // clang-format on

int builtin::match::builtin_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto session = lua->touserdata<fb::game::match>(1);
    if (session == nullptr)
        return 0;

    lua->pushstring(session->id());
    return 1;
}

int builtin::match::builtin_type(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto session = lua->touserdata<fb::game::match>(1);
    if (session == nullptr)
        return 0;

    lua->pushinteger(session->type());
    return 1;
}

int builtin::match::builtin_state(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto session = lua->touserdata<fb::game::match>(1);
    if (session == nullptr)
        return 0;

    switch (session->state())
    {
    case MATCH_STATE::waiting:
        lua->pushstring("waiting");
        break;
    case MATCH_STATE::playing:
        lua->pushstring("playing");
        break;
    case MATCH_STATE::ending:
        lua->pushstring("ending");
        break;
    case MATCH_STATE::closed:
        lua->pushstring("closed");
        break;
    default:
        lua->pushstring("waiting");
        break;
    }
    return 1;
}

int builtin::match::builtin_wait(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto session = lua->touserdata<fb::game::match>(1);
    if (session == nullptr)
        return 0;

    auto seconds = static_cast<uint32_t>(lua->tointeger(2));
    session->wait(seconds);
    return 0;
}

int builtin::match::builtin_finish(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto session = lua->touserdata<fb::game::match>(1);
    if (session == nullptr)
        return 0;

    auto grace = uint32_t{0};
    if (lua->argc() >= 2)
        grace = static_cast<uint32_t>(lua->tointeger(2));
    session->finish(grace);
    return 0;
}

int builtin::match::builtin_members(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto session = lua->touserdata<fb::game::match>(1);
    if (session == nullptr)
        return 0;

    auto members = session->members();
    lua->new_table();
    for (auto i = 0; i < static_cast<int>(members.size()); i++)
    {
        lua->pushobject(*members[i]);
        lua->rawseti(-2, i + 1);
    }
    return 1;
}

int builtin::match::builtin_member_count(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto session = lua->touserdata<fb::game::match>(1);
    if (session == nullptr)
        return 0;

    lua->pushinteger(session->member_count());
    return 1;
}

int builtin::match::builtin_expected(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto session = lua->touserdata<fb::game::match>(1);
    if (session == nullptr)
        return 0;

    lua->pushinteger(session->expected());
    return 1;
}

int builtin::match::builtin_map(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto session = lua->touserdata<fb::game::match>(1);
    if (session == nullptr)
        return 0;

    auto model_id = static_cast<uint32_t>(lua->tointeger(2));
    auto map      = session->map(model_id);
    if (map == nullptr)
        lua->pushnil();
    else
        lua->pushobject(*map);
    return 1;
}
