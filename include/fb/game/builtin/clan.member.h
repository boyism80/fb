#ifndef FB_GAME_BUILTIN_CLAN_MEMBER_H
#define FB_GAME_BUILTIN_CLAN_MEMBER_H

#include <fb/game/clan.member.h>

namespace fb::game::builtin {

struct clan_member
{
    static int builtin_name(lua_State* L);
    static int builtin_role(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_CLAN_MEMBER_H
