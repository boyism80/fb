#ifndef FB_GAME_HANDLER_NPC_INTERACTION_RENAME_WEAPON_H
#define FB_GAME_HANDLER_NPC_INTERACTION_RENAME_WEAPON_H

#include <fb/game/npc_interaction_handler.h>
#include <fb/model/model.h>
#include <boost/xpressive/xpressive.hpp>

namespace fb::game::handler::npc_interaction {

class rename_weapon : public fb::game::npc_interaction_handler
{
public:
    rename_weapon(server_type& server) :
        npc_interaction_handler(server)
    { }
    bool matches(const std::string& message) const override
    {
        static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::RENAME_WEAPON);
        return boost::xpressive::regex_search(message, regex);
    }

    async::task<void> handle(character_type& ch, const std::string& message, const npc_list_type& npcs) override;
};

} // namespace fb::game::handler::npc_interaction

#endif // FB_GAME_HANDLER_NPC_INTERACTION_RENAME_WEAPON_H