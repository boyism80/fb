#ifndef FB_GAME_HANDLER_ITEM_INACTIVE_H
#define FB_GAME_HANDLER_ITEM_INACTIVE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class item_inactive : public fb::handler::protocol<fb::game::server, fb::protocol::game::request::item_inactive>
{
public:
    item_inactive(fb::game::server& server);
    item_inactive(const item_inactive&)             = delete;
    item_inactive(item_inactive&&)                  = delete;
    item_inactive& operator= (const item_inactive&) = delete;
    item_inactive& operator= (item_inactive&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::item_inactive& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_ITEM_INACTIVE_H
