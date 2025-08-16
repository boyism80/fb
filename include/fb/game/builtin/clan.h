#ifndef FB_GAME_BUILTIN_CLAN_H
#define FB_GAME_BUILTIN_CLAN_H

#include <fb/game/clan.h>

namespace fb::game::builtin {

struct clan
{
    /**
     * @brief      Lua binding for getting the clan's name.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_name(lua_State* L);

    /**
     * @brief      Lua binding for getting all clan members.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_members(lua_State* L);

    /**
     * @brief      Lua binding for finding nearby clan members.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_nears(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting the clan title.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_title(lua_State* L);

    /**
     * @brief      Lua binding for adding a member to the clan.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_join(lua_State* L);

    /**
     * @brief      Lua binding for removing a member from the clan.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_leave(lua_State* L);

    /**
     * @brief      Lua binding for kicking a member from the clan.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_kick(lua_State* L);

    /**
     * @brief      Lua binding for changing the role of a member in the clan.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_change_role(lua_State* L);

    /**
     * @brief      Lua binding for sending messages to clan members.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_message(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_CLAN_H
