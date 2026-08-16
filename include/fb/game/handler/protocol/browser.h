#ifndef FB_GAME_HANDLER_PROTOCOL_BROWSER_H
#define FB_GAME_HANDLER_PROTOCOL_BROWSER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

/**
 * Handler for C2S browser (0x73) in-game IE window request (6.51).
 * Replies with S2C 0x62 browser payload.
 */
template <fb::protocol::CLIENT_VERSION V>
class browser : public fb::handler::protocol<fb::game::server, game_reqs::browser<V>>
{
public:
    browser(fb::game::server& server) :
        fb::handler::protocol<fb::game::server, game_reqs::browser<V>>(server)
    { }
    browser(const browser&)             = delete;
    browser(browser&&)                  = delete;
    browser& operator= (const browser&) = delete;
    browser& operator= (browser&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::browser<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif
