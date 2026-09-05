#ifndef FB_GAME_HANDLER_PROTOCOL_PONG_H
#define FB_GAME_HANDLER_PROTOCOL_PONG_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

/**
 * Handler for client pong (0x75). Sets pong_received on the character's ping state.
 */
template <fb::protocol::CLIENT_VERSION V>
class pong : public fb::handler::protocol<fb::game::server, game_reqs::pong<V>>
{
public:
    pong(fb::game::server& server) :
        fb::handler::protocol<fb::game::server, game_reqs::pong<V>>(server)
    { }
    pong(const pong&)             = delete;
    pong(pong&&)                  = delete;
    pong& operator= (const pong&) = delete;
    pong& operator= (pong&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::pong<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif
