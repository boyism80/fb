#ifndef FB_GAME_BUILTIN_MATCHMAKER_H
#define FB_GAME_BUILTIN_MATCHMAKER_H

#include <fb/game/matchmaker.h>

namespace fb::game::builtin {

struct matchmaker
{
    static int builtin_mu(lua_State* L);
    static int builtin_sigma(lua_State* L);
    static int builtin_enrolled(lua_State* L);
    static int builtin_registry_id(lua_State* L);
    static int builtin_pending_match_id(lua_State* L);
    static int builtin_register(lua_State* L);
    static int builtin_unregister(lua_State* L);
    static int builtin_confirm(lua_State* L);
    static int builtin_decline(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_MATCHMAKER_H
