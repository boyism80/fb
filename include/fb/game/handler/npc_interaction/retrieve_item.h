#ifndef FB_GAME_HANDLER_NPC_INTERACTION_RETRIEVE_ITEM_H
#define FB_GAME_HANDLER_NPC_INTERACTION_RETRIEVE_ITEM_H

#include <fb/game/npc_interaction_handler.h>
#include <fb/model/model.h>
#include <boost/xpressive/xpressive.hpp>

namespace fb::game::handler::npc_interaction {

/**
 * @brief   Handles NPC item retrieval interactions.
 *
 *          Processes player messages for item retrieval with NPCs.
 */
class retrieve_item : public fb::game::npc_interaction_handler
{
public:
    /**
     * @brief   Constructs a retrieve_item handler.
     *
     * @param[in]  server  Reference to the game server
     */
    retrieve_item(server_type& server) : npc_interaction_handler(server) {}

    /**
     * @brief   Checks if the message matches the retrieve_item pattern.
     *
     * @param[in]  message  The user input message to check
     * @return     True if the message matches the pattern, false otherwise
     */
    bool matches(const std::string& message) const override
    {
        static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::RETRIEVE_ITEM);
        return boost::xpressive::regex_search(message, regex);
    }

    /**
     * @brief   Handles item retrieval interaction.
     *
     * @param[in]  ch       The character performing the interaction
     * @param[in]  message  The interaction message from the player
     * @param[in]  npcs     The list of nearby NPCs
     */
    async::task<void> handle(character_type& ch, 
                           const std::string& message,
                           const npc_list_type& npcs) override;
};

} // namespace fb::game::handler::npc_interaction

#endif // FB_GAME_HANDLER_NPC_INTERACTION_RETRIEVE_ITEM_H