#ifndef FB_GAME_HANDLER_NPC_INTERACTION_STORE_ITEM_H
#define FB_GAME_HANDLER_NPC_INTERACTION_STORE_ITEM_H

#include <fb/game/npc_interaction_handler.h>
#include <fb/model/model.h>
#include <boost/xpressive/xpressive.hpp>

namespace fb::game::handler::npc_interaction {

class store_item : public fb::game::npc_interaction_handler
{
public:
    store_item(server_type& server) :
        npc_interaction_handler(server)
    { }
    bool matches(const std::string& message) const override
    {
        static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::STORE_ITEM);
        return boost::xpressive::regex_search(message, regex);
    }

    async::task<void> handle(character_type& ch, const std::string& message, const npc_list_type& npcs) override;
};

} // namespace fb::game::handler::npc_interaction

#endif // FB_GAME_HANDLER_NPC_INTERACTION_STORE_ITEM_H