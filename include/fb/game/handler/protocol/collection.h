#ifndef FB_GAME_HANDLER_PROTOCOL_COLLECTION_H
#define FB_GAME_HANDLER_PROTOCOL_COLLECTION_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class collection : public fb::handler::protocol<fb::game::server, game_reqs::collection<V>>
{
public:
    collection(fb::game::server& server) :
        fb::handler::protocol<fb::game::server, game_reqs::collection<V>>(server)
    { }
    collection(const collection&)             = delete;
    collection(collection&&)                  = delete;
    collection& operator= (const collection&) = delete;
    collection& operator= (collection&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::collection<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif
