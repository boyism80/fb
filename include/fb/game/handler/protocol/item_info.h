#ifndef FB_GAME_HANDLER_ITEM_INFO_H
#define FB_GAME_HANDLER_ITEM_INFO_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class item_info : public fb::handler::protocol<fb::game::server, game_reqs::item_info>
{
public:
    item_info(fb::game::server& server);
    item_info(const item_info&)             = delete;
    item_info(item_info&&)                  = delete;
    item_info& operator= (const item_info&) = delete;
    item_info& operator= (item_info&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::item_info& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_ITEM_INFO_H
