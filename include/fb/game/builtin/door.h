#ifndef FB_GAME_BUILTIN_DOOR_H
#define FB_GAME_BUILTIN_DOOR_H

#include <fb/game/door.h>

namespace fb::game::builtin {

struct door
{
    /**
     * @brief      Lua binding for toggling door open/closed state.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_toggle(lua_State* L);

    /**
     * @brief      Lua binding for checking if door is locked.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_locked(lua_State* L);

    /**
     * @brief      Lua binding for setting door lock state.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_lock(lua_State* L);

    /**
     * @brief      Lua binding for checking if door is opened.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_opened(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_DOOR_H
