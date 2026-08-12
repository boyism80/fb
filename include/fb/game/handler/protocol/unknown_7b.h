#ifndef FB_GAME_HANDLER_PROTOCOL_UNKNOWN_7B_H
#define FB_GAME_HANDLER_PROTOCOL_UNKNOWN_7B_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

/**
 * Handler for C2S unknown_7b (0x7B), a name miss report for S2C unknown_6f (0x6F).
 * The reported name is only logged until the name list feature is implemented.
 */
template <fb::protocol::CLIENT_VERSION V>
class unknown_7b : public fb::handler::protocol<fb::game::server, game_reqs::unknown_7b<V>>
{
public:
    unknown_7b(fb::game::server& server) :
        fb::handler::protocol<fb::game::server, game_reqs::unknown_7b<V>>(server)
    { }
    unknown_7b(const unknown_7b&)             = delete;
    unknown_7b(unknown_7b&&)                  = delete;
    unknown_7b& operator= (const unknown_7b&) = delete;
    unknown_7b& operator= (unknown_7b&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::unknown_7b<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif
