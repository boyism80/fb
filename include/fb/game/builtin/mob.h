#ifndef FB_GAME_BUILTIN_MOB_H
#define FB_GAME_BUILTIN_MOB_H

#include <fb/game/mob.h>

namespace fb::game::builtin {

struct mob
{
    /**
     * @brief      Lua binding to get or set the mob's current target.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_target(lua_State* L);

    /**
     * @brief      Lua binding to get or set the mob's oblivion target (last attacker).
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_oblivion(lua_State* L);

    /**
     * @brief      Lua binding to get the mob's owner (if it's a spawned mob).
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_owner(lua_State* L);

    /**
     * @brief      Lua binding to get the mob's item inventory.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_items(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_MOB_H
