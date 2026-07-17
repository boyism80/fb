#ifndef FB_GAME_BUILTIN_MOB_H
#define FB_GAME_BUILTIN_MOB_H

#include <fb/game/mob.h>

namespace fb::game::builtin {

struct mob
{
    static int builtin_target(lua_State* L);
    static int builtin_oblivion(lua_State* L);
    static int builtin_owner(lua_State* L);
    static int builtin_items(lua_State* L);
    static int builtin_parts(lua_State* L);
    static int builtin_body(lua_State* L);
    static int builtin_parts_mode(lua_State* L);
    static int builtin_hide(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_MOB_H
