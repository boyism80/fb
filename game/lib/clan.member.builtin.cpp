#include <fb/game/clan.member.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::clan_member, "fb.game.clan.member")
{"name",                fb::game::clan_member::builtin_name},
{"position",            fb::game::clan_member::builtin_position},
END_LUA_EXTENSION; // clang-format on

int clan_member::builtin_name(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto member = thread->touserdata<fb::game::clan_member>(1);
    if (member == nullptr)
        return 0;

    thread->pushstring(member->name);
    return 1;
}

int clan_member::builtin_position(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto member = thread->touserdata<fb::game::clan_member>(1);
    if (member == nullptr)
        return 0;

    thread->pushinteger(static_cast<uint32_t>(member->position));
    return 1;
}