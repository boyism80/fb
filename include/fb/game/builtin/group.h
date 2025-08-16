#ifndef FB_GAME_BUILTIN_GROUP_H
#define FB_GAME_BUILTIN_GROUP_H

#include <fb/game/group.h>

namespace fb::game::builtin {

struct group
{
    /**
     * @brief      Lua binding for getting the group master's name.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_master(lua_State* L);

    /**
     * @brief      Lua binding for getting all group members.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_members(lua_State* L);

    /**
     * @brief      Lua binding for finding nearby group members.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_nears(lua_State* L);

    /**
     * @brief      Lua binding for sending messages to group members.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_message(lua_State* L);

    /**
     * @brief      Lua binding for kicking a member from the group.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_kick(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_GROUP_H
