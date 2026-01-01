#ifndef FB_GAME_HANDLER_ITEM_THROWS_H
#define FB_GAME_HANDLER_ITEM_THROWS_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class item_throws : public fb::handler::protocol<fb::game::server, game_reqs::item_throws>
{
public:
    item_throws(fb::game::server& server);
    item_throws(const item_throws&)             = delete;
    item_throws(item_throws&&)                  = delete;
    item_throws& operator= (const item_throws&) = delete;
    item_throws& operator= (item_throws&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::item_throws& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_ITEM_THROWS_H
