#ifndef FB_GAME_HANDLER_NPC_INTERACTION_STORE_ITEM_COUNT_H
#define FB_GAME_HANDLER_NPC_INTERACTION_STORE_ITEM_COUNT_H

#include <fb/game/npc_interaction_handler.h>
#include <fb/model/model.h>
#include <boost/xpressive/xpressive.hpp>

namespace fb::game::handler::npc_interaction {

/**
 * @brief   Handles NPC stored item count inquiry interactions.
 *
 *          Processes player messages for stored item count inquiry with NPCs.
 */
class store_item_count : public fb::game::npc_interaction_handler
{
public:
    /**
     * @brief   Constructs a store_item_count handler.
     *
     * @param[in]  server  Reference to the game server
     */
    store_item_count(server_type& server) : npc_interaction_handler(server) {}

    /**
     * @brief   Checks if the message matches the store_item_count pattern.
     *
     * @param[in]  message  The user input message to check
     * @return     True if the message matches the pattern, false otherwise
     */
    bool matches(const std::string& message) const override
    {
        static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::HOLD_ITEM_COUNT);
        return boost::xpressive::regex_search(message, regex);
    }

    /**
     * @brief   Handles stored item count inquiry interaction.
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

#endif // FB_GAME_HANDLER_NPC_INTERACTION_STORE_ITEM_COUNT_H