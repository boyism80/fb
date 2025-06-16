#ifndef __CLAN_MEMBER_H__
#define __CLAN_MEMBER_H__

#include <fb/lua.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::game {

/**
 * @brief      Represents a member of a clan with position and Lua scripting support.
 *
 *             This class encapsulates information about a clan member, including their
 *             name and position within the clan hierarchy. It extends the Lua integration
 *             system to allow clan member data to be accessed and manipulated from
 *             Lua scripts, enabling flexible clan management and scripting capabilities.
 *
 *             Key features:
 *             - Member name and clan position tracking
 *             - Lua scripting integration for dynamic clan management
 *             - Built-in Lua functions for accessing member properties
 *             - Integration with the clan hierarchy system
 */
class clan_member : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    struct builtin;

public:
    std::string   name;     ///< The member's character name
    CLAN_POSITION position; ///< The member's position within the clan hierarchy

public:
    /**
     * @brief      Constructs a new clan member with the specified name and position.
     *
     *             Creates a clan member object with the given character name and
     *             clan position, ready for integration into the clan system.
     *
     * @param[in]  name      The character name of the clan member.
     * @param[in]  position  The member's position within the clan hierarchy.
     */
    clan_member(const std::string& name, CLAN_POSITION position);

    /**
     * @brief      Destroys the clan member and cleans up resources.
     */
    ~clan_member() = default;
};

/**
 * @brief      Built-in Lua functions for clan member scripting integration.
 *
 *             This structure contains static functions that provide Lua script
 *             access to clan member properties and operations. These functions
 *             are registered with the Lua interpreter to enable script-based
 *             clan member management and queries.
 */
struct clan_member::builtin
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
     * @brief      Lua function to get the clan member's position.
     *
     *             Returns the clan position of the member to Lua scripts,
     *             enabling position-based logic and hierarchy management.
     *
     * @param      L   The Lua state for script execution.
     *
     * @return     Number of return values pushed to the Lua stack (1 for position).
     */
    static int builtin_position(lua_State* L);
};

} // namespace fb::game

#endif