#ifndef __GROUP_H__
#define __GROUP_H__

#include <fb/game/character.h>
#include <async/task.h>

namespace fb::game {

/**
 * @brief      Represents a player group for cooperative gameplay.
 *
 *             This class manages a group of players who have joined together for cooperative
 *             gameplay activities. It handles group membership, master/leader designation,
 *             member synchronization, and provides group-specific functionality such as
 *             experience sharing and proximity-based operations.
 *
 *             Key features:
 *             - Group membership management with master/member roles
 *             - Thread-safe operations with context switching
 *             - Member synchronization across server instances
 *             - Proximity-based member queries for area effects
 *             - Lua scripting integration for group-based logic
 *             - Real-time member status tracking
 */
class group : public fb::thread_switchable
{
public:
    LUA_PROTOTYPE

public:
    struct builtin;

private:
    context&                 _context;
    uint32_t                 _id;
    std::string              _master;
    std::vector<std::string> _members;
    std::vector<character*>  _active_members;

public:
    /**
     * @brief      Constructs a new player group with the specified context and identifier.
     *
     *             Creates a new group object managed by the given game context.
     *             The group starts empty and must be populated with members through
     *             the update() method or by characters entering individually.
     *
     * @param[in]  context  The game context that manages this group
     * @param[in]  id       The unique identifier for this group
     */
    group(context& context, uint32_t id);

    /**
     * @brief      Copy constructor (deleted).
     *
     *             Groups cannot be copied to prevent resource management issues
     *             and maintain unique group identity within the game context.
     */
    group(const group&) = delete;

    /**
     * @brief      Move constructor for transferring group ownership.
     *
     *             Allows moving a group object while preserving all member data,
     *             master assignment, and active character associations.
     *
     * @param[in]  g     The source group object to move from
     */
    group(group&& g);

    /**
     * @brief      Destroys the group and cleans up member associations.
     *
     *             Removes all active character associations and cleans up group
     *             resources when the group object is destroyed.
     */
    ~group() = default;

public:
    /**
     * @brief      Adds a character to the group as an active member.
     *
     * @param      ch    The character to add to the group
     */
    void enter(character& ch);

    /**
     * @brief      Removes a character from the group's active members.
     *
     * @param      ch    The character to remove from the group
     */
    void leave(character& ch);

    /**
     * @brief      Updates the group's master and member list from external data.
     *
     * @param[in]  master   The name of the new group master/leader
     * @param[in]  members  The updated list of member names
     *
     * @return     Async task that completes when the update is finished
     */
    async::task<void> update(const std::string& master, const std::vector<std::string>& members);

    /**
     * @brief      Gets the unique identifier of this group.
     *
     * @return     The group's unique ID
     */
    uint32_t id() const;

    /**
     * @brief      Checks if the group has been fully initialized.
     *
     * @return     True if the group is initialized, false otherwise
     */
    bool inited() const;

    /**
     * @brief      Gets the name of the group master/leader.
     *
     * @return     Reference to the master's name string
     */
    const std::string& master() const;

    /**
     * @brief      Gets all currently active character members in the group.
     *
     * @return     Vector of pointers to active character members
     */
    std::vector<character*> characters() const;

    /**
     * @brief      Gets the names of all group members (active and inactive).
     *
     * @return     Vector of member name strings
     */
    std::vector<std::string> members() const;

    /**
     * @brief      Finds group members near a specific position on a map.
     *
     * @param[in]  map       The map to search on
     * @param[in]  position  The center position to search around
     *
     * @return     Vector of character pointers for nearby group members
     */
    std::vector<character*> nears(const fb::game::map& map, const fb::model::point16_t& position) const;

    /**
     * @brief      Gets the thread that manages this group's execution context.
     *
     * @return     Pointer to the thread managing this group
     */
    fb::thread* thread() const override;
};

struct group::builtin
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
};

} // namespace fb::game

#endif // !__GROUP_H__
