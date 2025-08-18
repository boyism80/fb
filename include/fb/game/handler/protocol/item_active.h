#ifndef FB_GAME_HANDLER_ITEM_ACTIVE_H
#define FB_GAME_HANDLER_ITEM_ACTIVE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class item_active : public fb::handler::protocol<fb::game::server, fb::protocol::game::request::item_active>
{
public:
    item_active(fb::game::server& server);
    item_active(const item_active&)             = delete;
    item_active(item_active&&)                  = delete;
    item_active& operator= (const item_active&) = delete;
    item_active& operator= (item_active&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::item_active& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_ITEM_ACTIVE_H
