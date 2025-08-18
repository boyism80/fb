#ifndef FB_GAME_HANDLER_NPC_INTERACTION_REVIVE_H
#define FB_GAME_HANDLER_NPC_INTERACTION_REVIVE_H

#include <fb/game/npc_interaction_handler.h>
#include <fb/model/model.h>
#include <boost/xpressive/xpressive.hpp>

namespace fb::game::handler::npc_interaction {

class revive : public fb::game::npc_interaction_handler
{
public:
    revive(server_type& server) :
        npc_interaction_handler(server)
    { }
    bool matches(const std::string& message) const override
    {
        static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::REVIVE);
        return boost::xpressive::regex_search(message, regex);
    }

    async::task<void> handle(character_type& ch, const std::string& message, const npc_list_type& npcs) override;
};

} // namespace fb::game::handler::npc_interaction

#endif // __FB_GAME_HANDLER_NPC_INTERACTION_REVIVE_H__