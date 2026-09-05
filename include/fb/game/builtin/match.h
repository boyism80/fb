#ifndef FB_GAME_BUILTIN_MATCH_H
#define FB_GAME_BUILTIN_MATCH_H

#include <fb/game/match.h>

namespace fb::game::builtin {

struct match
{
    static int builtin_id(lua_State* L);
    static int builtin_type(lua_State* L);
    static int builtin_state(lua_State* L);
    static int builtin_wait(lua_State* L);
    static int builtin_duration(lua_State* L);
    static int builtin_finish(lua_State* L);
    static int builtin_members(lua_State* L);
    static int builtin_member_count(lua_State* L);
    static int builtin_expected(lua_State* L);
    static int builtin_map(lua_State* L);
};

} // namespace fb::game::builtin

#endif
