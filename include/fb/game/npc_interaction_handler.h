#ifndef FB_GAME_NPC_INTERACTION_HANDLER_H
#define FB_GAME_NPC_INTERACTION_HANDLER_H

#include <async/task.h>
#include <memory>
#include <vector>
#include <string>

namespace fb::game {

// Forward declarations
class character;
class npc;
class server;

/**
 * @brief   Abstract base class for all NPC interaction handlers.
 *
 *          This class provides a common interface for all NPC interaction handlers,
 *          enabling polymorphic storage and execution of different handler types.
 */
class npc_interaction_handler
{
public:
    using character_type = fb::game::character;
    using npc_list_type  = std::vector<std::shared_ptr<fb::game::npc>>;
    using server_type    = fb::game::server;

protected:
    server_type& server;

protected:
    npc_interaction_handler(server_type& server) :
        server(server)
    { }

public:
    virtual ~npc_interaction_handler() = default;

    /**
     * @brief   Checks if the message matches this handler's pattern.
     *
     * @param[in]  message  The user input message to check
     * @return     True if the message matches the pattern, false otherwise
     */
    virtual bool matches(const std::string& message) const = 0;

    /**
     * @brief   Handles NPC interaction for matching messages.
     *
     * @param[in]  ch       The character performing the interaction
     * @param[in]  message  The interaction message from the player
     * @param[in]  npcs     The list of nearby NPCs
     */
    virtual async::task<void> handle(character_type& ch, const std::string& message, const npc_list_type& npcs) = 0;
};

} // namespace fb::game

#endif // FB_GAME_NPC_INTERACTION_HANDLER_H