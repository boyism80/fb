#ifndef __CLAN_MEMBER_H__
#define __CLAN_MEMBER_H__

#include <fb/lua.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::game {

/**
 * @brief      Represents a member of a clan with role and Lua scripting support.
 *
 *             This class encapsulates information about a clan member, including their
 *             name and role within the clan hierarchy. It extends the Lua integration
 *             system to allow clan member data to be accessed and manipulated from
 *             Lua scripts, enabling flexible clan management and scripting capabilities.
 *
 *             Key features:
 *             - Member name and clan role tracking
 *             - Lua scripting integration for dynamic clan management
 *             - Built-in Lua functions for accessing member properties
 *             - Integration with the clan hierarchy system
 */
class clan_member : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    std::string name; ///< The member's character name
    CLAN_ROLE   role; ///< The member's role within the clan hierarchy

public:
    /**
     * @brief      Constructs a new clan member with the specified name and role.
     *
     *             Creates a clan member object with the given character name and
     *             clan role, ready for integration into the clan system.
     *
     * @param[in]  name      The character name of the clan member.
     * @param[in]  role  The member's role within the clan hierarchy.
     */
    clan_member(const std::string& name, CLAN_ROLE role);

    /**
     * @brief      Destroys the clan member and cleans up resources.
     */
    ~clan_member() = default;
};

} // namespace fb::game

#endif