#ifndef FB_GAME_BUILTIN_GROUP_H
#define FB_GAME_BUILTIN_GROUP_H

#include <fb/game/group.h>

namespace fb::game::builtin {

struct group
{
    static int builtin_master(lua_State* L);
    static int builtin_members(lua_State* L);
    static int builtin_nears(lua_State* L);
    static int builtin_message(lua_State* L);
    static int builtin_kick(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_GROUP_H
