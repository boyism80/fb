#ifndef FB_GAME_HANDLER_PROTOCOL_WEB_MAP_H
#define FB_GAME_HANDLER_PROTOCOL_WEB_MAP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

/**
 * Handler for C2S web_map (0x7C) web map request (6.51).
 * Replies with S2C 0x70 markers for other characters on the current map.
 */
template <fb::protocol::CLIENT_VERSION V>
class web_map : public fb::handler::protocol<fb::game::server, game_reqs::web_map<V>>
{
public:
    web_map(fb::game::server& server) :
        fb::handler::protocol<fb::game::server, game_reqs::web_map<V>>(server)
    { }
    web_map(const web_map&)             = delete;
    web_map(web_map&&)                  = delete;
    web_map& operator= (const web_map&) = delete;
    web_map& operator= (web_map&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::web_map<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif
