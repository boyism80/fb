#ifndef FB_GAME_BUILTIN_CASTLE_H
#define FB_GAME_BUILTIN_CASTLE_H

#include <fb/game/castle.h>

namespace fb::game::builtin {

struct castle
{
    static int builtin_divine_beast(lua_State* L);
    static int builtin_owner_clan(lua_State* L);
    static int builtin_siege_active(lua_State* L);
    static int builtin_siege_end_time(lua_State* L);
    static int builtin_start_siege(lua_State* L);
    static int builtin_end_siege(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_CASTLE_H
