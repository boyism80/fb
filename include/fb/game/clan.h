#ifndef __CLAN_H__
#define __CLAN_H__

/**
 * @file    clan.h
 * @brief   Player clan (guild) system for long-term cooperative gameplay
 * @author  FB Development Team
 *
 * @details This file implements the clan (guild) system that allows players to form
 *          permanent organizations for long-term cooperative gameplay and social
 *          interaction. Clans provide persistent group identity, hierarchical
 *          structure, and various social and gameplay benefits.
 *
 *          Key features:
 *          - Persistent clan membership with hierarchical roles and ranks
 *          - Clan naming and title/motto system for identity and recognition
 *          - Real-time member tracking and online status management
 *          - Cross-server member synchronization for distributed game architecture
 *          - Proximity-based member queries for area effects and clan spells
 *          - Clan-wide communication and messaging systems
 *          - Member management with join/leave functionality
 *          - Integration with character system for persistent clan associations
 *          - Lua scripting integration for clan-based game logic and events
 *          - Thread-safe operations for multi-threaded server environment
 *
 * @note    Clans are permanent organizations that persist beyond individual
 *          play sessions, unlike groups which are temporary associations.
 */

#include <fb/game/character.h>
#include <fb/game/clan.member.h>

namespace fb::game {

/**
 * @brief      Represents a player clan for guild-based gameplay.
 *
 *             This class manages a clan (guild) of players who have joined together for
 *             long-term cooperative gameplay and social interaction. It handles clan
 *             membership, hierarchy, titles, and provides clan-specific functionality
 *             such as member management and clan-wide communication.
 *
 *             Key features:
 *             - Clan membership management with hierarchical roles
 *             - Clan title and naming system
 *             - Member synchronization across server instances
 *             - Real-time member tracking and status
 *             - Proximity-based member queries for area effects
 *             - Lua scripting integration for clan-based logic
 *             - Persistent clan data management
 */
class clan : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    struct builtin;

private:
    context&                                                         _context;
    uint32_t                                                         _id;
    std::string                                                      _name;
    std::optional<std::string>                                       _title;
    std::unordered_map<std::string, clan_member>                     _members;
    std::unordered_map<uint32_t, std::weak_ptr<fb::game::character>> _characters;

public:
    /**
     * @brief      Constructs a new clan with the specified context and identifier.
     *
     *             Creates a new clan object managed by the given game context.
     *             The clan starts with no members and must be populated through
     *             the update() method or by adding members individually.
     *
     * @param[in]  context  The game context that manages this clan
     * @param[in]  id       The unique identifier for this clan
     */
    clan(context& context, uint32_t id, const std::string& name, const std::optional<std::string>& title);

    /**
     * @brief      Copy constructor (deleted).
     *
     *             Clans cannot be copied to prevent resource management issues
     *             and maintain unique clan identity within the game context.
     */
    clan(const clan&) = delete;

    /**
     * @brief      Move constructor for transferring clan ownership.
     *
     *             Allows moving a clan object while preserving all member data,
     *             character associations, and clan properties.
     */
    clan(clan&&);

    /**
     * @brief      Destroys the clan and cleans up member associations.
     *
     *             Detaches all associated characters and cleans up clan resources
     *             when the clan object is destroyed.
     */
    ~clan() = default;

public:
    /**
     * @brief      Updates the clan's name, title, and member list from external data.
     *
     * @param[in]  name     The new clan name
     * @param[in]  title    The optional clan title/motto
     * @param[in]  members  The updated list of clan members
     */
    void update(const std::string&                name,
                const std::optional<std::string>& title,
                const std::vector<clan_member>&   members);

    /**
     * @brief      Gets the unique identifier of this clan.
     *
     * @return     The clan's unique ID
     */
    uint32_t id() const;

    /**
     * @brief      Gets the name of the clan.
     *
     * @return     Reference to the clan's name string
     */
    const std::string& name() const;
    /**
     * @brief      Gets the clan's title/motto.
     *
     * @return     Reference to the optional clan title string
     */
    const std::optional<std::string>& title() const;

    /**
     * @brief      Sets the clan's title/motto.
     *
     * @param[in]  title  The new clan title to set
     */
    void title(const std::optional<std::string>& title);

    /**
     * @brief      Gets all clan members with their roles and information.
     *
     * @return     Reference to the map of member names to clan member data
     */
    const std::unordered_map<std::string, clan_member>& members() const;

    /**
     * @brief      Adds a new member to the clan.
     *
     * @param[in]  member  The clan member data to add
     */
    void join(const clan_member& member);

    /**
     * @brief      Removes a member from the clan by name.
     *
     * @param[in]  member  The name of the member to remove
     */
    void leave(const std::string& member);

    /**
     * @brief      Gets all currently online clan member characters.
     *
     * @return     Reference to the map of character IDs to character pointers
     */
    const std::unordered_map<uint32_t, std::weak_ptr<fb::game::character>>& characters() const;

    /**
     * @brief      Attaches a character to the clan as an active member.
     *
     * @param      ch    The character to attach to the clan
     */
    void attach_character(std::weak_ptr<character> ch);
    /**
     * @brief      Detaches a character from the clan's active members.
     *
     * @param      ch    The character to detach from the clan
     */
    void detach_character(std::weak_ptr<character> ch);

    /**
     * @brief      Finds clan members near a specific position on a map.
     *
     * @param[in]  map       The map to search on
     * @param[in]  position  The center position to search around
     *
     * @return     Vector of character pointers for nearby clan members
     */
    std::vector<std::shared_ptr<fb::game::character>> nears(const fb::game::map&        map,
                                                            const fb::model::point16_t& position) const;
};

struct clan::builtin
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
     * @brief      Lua binding for sending messages to clan members.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_message(lua_State* L);
};

} // namespace fb::game

#endif // !__CLAN_H__
