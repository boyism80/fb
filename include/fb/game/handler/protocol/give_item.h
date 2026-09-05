#ifndef FB_GAME_HANDLER_GIVE_ITEM_H
#define FB_GAME_HANDLER_GIVE_ITEM_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class give_item : public fb::handler::protocol<fb::game::server, game_reqs::give_item<V>>
{
public:
    give_item(fb::game::server& server);
    give_item(const give_item&)             = delete;
    give_item(give_item&&)                  = delete;
    give_item& operator= (const give_item&) = delete;
    give_item& operator= (give_item&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::give_item<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_GIVE_ITEM_H
