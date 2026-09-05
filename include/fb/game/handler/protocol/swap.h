#ifndef FB_GAME_HANDLER_SWAP_H
#define FB_GAME_HANDLER_SWAP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class swap : public fb::handler::protocol<fb::game::server, game_reqs::swap<V>>
{
public:
    swap(fb::game::server& server);
    swap(const swap&)             = delete;
    swap(swap&&)                  = delete;
    swap& operator= (const swap&) = delete;
    swap& operator= (swap&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::swap<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_SWAP_H
