#ifndef FB_GAME_BUILTIN_CLAN_MEMBER_H
#define FB_GAME_BUILTIN_CLAN_MEMBER_H

#include <fb/game/clan.member.h>

namespace fb::game::builtin {

struct clan_member
{
    /**
     * @brief      Lua function to get the clan member's name.
     *
     *             Returns the character name of the clan member to Lua scripts,
     *             allowing scripts to identify and work with specific members.
     *
     * @param      L   The Lua state for script execution.
     *
     * @return     Number of return values pushed to the Lua stack (1 for name).
     */
    static int builtin_name(lua_State* L);

    /**
     * @brief      Lua function to get the clan member's role.
     *
     *             Returns the clan role of the member to Lua scripts,
     *             enabling role-based logic and hierarchy management.
     *
     * @param      L   The Lua state for script execution.
     *
     * @return     Number of return values pushed to the Lua stack (1 for role).
     */
    static int builtin_role(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_CLAN_MEMBER_H
