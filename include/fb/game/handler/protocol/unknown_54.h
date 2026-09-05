#ifndef FB_GAME_HANDLER_PROTOCOL_UNKNOWN_54_H
#define FB_GAME_HANDLER_PROTOCOL_UNKNOWN_54_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

/**
 * Handler for C2S unknown_54 (0x54), the round trip of S2C unknown_4f (0x4F).
 * phase 5 is the window creation ack; no server side behaviour is known yet.
 */
template <fb::protocol::CLIENT_VERSION V>
class unknown_54 : public fb::handler::protocol<fb::game::server, game_reqs::unknown_54<V>>
{
public:
    unknown_54(fb::game::server& server) :
        fb::handler::protocol<fb::game::server, game_reqs::unknown_54<V>>(server)
    { }
    unknown_54(const unknown_54&)             = delete;
    unknown_54(unknown_54&&)                  = delete;
    unknown_54& operator= (const unknown_54&) = delete;
    unknown_54& operator= (unknown_54&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::unknown_54<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif
