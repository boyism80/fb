#ifndef FB_GAME_HANDLER_NPC_INTERACTION_SELL_H
#define FB_GAME_HANDLER_NPC_INTERACTION_SELL_H

#include <fb/game/npc_interaction_handler.h>
#include <fb/model/model.h>
#include <boost/xpressive/xpressive.hpp>

namespace fb::game::handler::npc_interaction {

/**
 * @brief   Handles NPC item selling interactions.
 *
 *          Processes player messages for selling items to NPCs. Extracts item name
 *          and optional quantity from the message and executes the sell transaction.
 */
class sell : public fb::game::npc_interaction_handler
{
public:
    /**
     * @brief   Constructs a sell handler.
     *
     * @param[in]  server  Reference to the game server
     */
    sell(server_type& server) :
        npc_interaction_handler(server)
    { }

    /**
     * @brief   Checks if the message matches the sell pattern.
     *
     * @param[in]  message  The user input message to check
     * @return     True if the message matches the pattern, false otherwise
     */
    bool matches(const std::string& message) const override
    {
        static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::SELL);
        return boost::xpressive::regex_search(message, regex);
    }

    /**
     * @brief   Handles item selling interaction.
     *
     * @param[in]  ch       The character attempting to sell items
     * @param[in]  message  The sell command message from the player
     * @param[in]  npcs     The list of nearby NPCs
     */
    async::task<void> handle(character_type& ch, const std::string& message, const npc_list_type& npcs) override;
};

} // namespace fb::game::handler::npc_interaction

#endif // FB_GAME_HANDLER_NPC_INTERACTION_SELL_H