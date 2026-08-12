#ifndef FB_GAME_HANDLER_PROTOCOL_UNKNOWN_27_H
#define FB_GAME_HANDLER_PROTOCOL_UNKNOWN_27_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

/**
 * Handler for C2S unknown_27 (0x27) list UI slot click.
 * The owning screen is unidentified, so the slot is only logged for now.
 */
template <fb::protocol::CLIENT_VERSION V>
class unknown_27 : public fb::handler::protocol<fb::game::server, game_reqs::unknown_27<V>>
{
public:
    unknown_27(fb::game::server& server) :
        fb::handler::protocol<fb::game::server, game_reqs::unknown_27<V>>(server)
    { }
    unknown_27(const unknown_27&)             = delete;
    unknown_27(unknown_27&&)                  = delete;
    unknown_27& operator= (const unknown_27&) = delete;
    unknown_27& operator= (unknown_27&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::unknown_27<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif
