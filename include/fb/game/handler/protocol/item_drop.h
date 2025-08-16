#ifndef FB_GAME_HANDLER_ITEM_DROP_H
#define FB_GAME_HANDLER_ITEM_DROP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class item_drop : public fb::handler::protocol<fb::game::server, fb::protocol::game::request::item_drop>
{
public:
    item_drop(fb::game::server& server);
    item_drop(const item_drop&)             = delete;
    item_drop(item_drop&&)                  = delete;
    item_drop& operator= (const item_drop&) = delete;
    item_drop& operator= (item_drop&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::item_drop& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_ITEM_DROP_H
