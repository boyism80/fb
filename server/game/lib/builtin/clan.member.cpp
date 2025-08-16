#include <fb/game/clan.member.h>
#include <fb/game/builtin/clan.member.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(clan_member, "fb.game.clan.member")
{"name",                builtin::clan_member::builtin_name},
{"role",                builtin::clan_member::builtin_role},
END_LUA_EXTENSION; // clang-format on

int builtin::clan_member::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto member = lua->touserdata<fb::game::clan_member>(1);
    if (member == nullptr)
        return 0;

    lua->pushstring(member->name);
    return 1;
}

int builtin::clan_member::builtin_role(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto member = lua->touserdata<fb::game::clan_member>(1);
    if (member == nullptr)
        return 0;

    lua->pushinteger(static_cast<uint32_t>(member->role));
    return 1;
}