#ifndef FB_GAME_HANDLER_NPC_INTERACTION_RENAME_WEAPON_H
#define FB_GAME_HANDLER_NPC_INTERACTION_RENAME_WEAPON_H

#include <fb/game/npc_interaction_handler.h>
#include <fb/model/model.h>
#include <boost/xpressive/xpressive.hpp>

namespace fb::game::handler::npc_interaction {

/**
 * @brief   Handles NPC weapon renaming interactions.
 *
 *          Processes player messages for weapon renaming with NPCs.
 */
class rename_weapon : public fb::game::npc_interaction_handler
{
public:
    /**
     * @brief   Constructs a rename_weapon handler.
     *
     * @param[in]  server  Reference to the game server
     */
    rename_weapon(server_type& server) : npc_interaction_handler(server) {}

    /**
     * @brief   Checks if the message matches the rename_weapon pattern.
     *
     * @param[in]  message  The user input message to check
     * @return     True if the message matches the pattern, false otherwise
     */
    bool matches(const std::string& message) const override
    {
        static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::RENAME_WEAPON);
        return boost::xpressive::regex_search(message, regex);
    }

    /**
     * @brief   Handles weapon renaming interaction.
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

#endif // FB_GAME_HANDLER_NPC_INTERACTION_RENAME_WEAPON_H