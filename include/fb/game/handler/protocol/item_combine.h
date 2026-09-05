#ifndef FB_GAME_HANDLER_ITEM_COMBINE_H
#define FB_GAME_HANDLER_ITEM_COMBINE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class item_combine : public fb::handler::protocol<fb::game::server, game_reqs::item_combine<V>>
{
public:
    item_combine(fb::game::server& server);
    item_combine(const item_combine&)             = delete;
    item_combine(item_combine&&)                  = delete;
    item_combine& operator= (const item_combine&) = delete;
    item_combine& operator= (item_combine&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::item_combine<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_ITEM_COMBINE_H
