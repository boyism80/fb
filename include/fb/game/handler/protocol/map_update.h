#ifndef FB_GAME_HANDLER_MAP_UPDATE_H
#define FB_GAME_HANDLER_MAP_UPDATE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class map_update : public fb::handler::protocol<fb::game::server, game_reqs::map_update>
{
public:
    map_update(fb::game::server& server);
    map_update(const map_update&)             = delete;
    map_update(map_update&&)                  = delete;
    map_update& operator= (const map_update&) = delete;
    map_update& operator= (map_update&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::map_update& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_MAP_UPDATE_H
