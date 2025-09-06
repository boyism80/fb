#ifndef FB_GAME_BUILTIN_CLAN_H
#define FB_GAME_BUILTIN_CLAN_H

#include <fb/game/clan.h>

namespace fb::game::builtin {

struct clan
{
    static int builtin_name(lua_State* L);
    static int builtin_members(lua_State* L);
    static int builtin_nears(lua_State* L);
    static int builtin_title(lua_State* L);
    static int builtin_join(lua_State* L);
    static int builtin_leave(lua_State* L);
    static int builtin_kick(lua_State* L);
    static int builtin_change_role(lua_State* L);
    static int builtin_message(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_CLAN_H
