#ifndef FB_GAME_HANDLER_MAP_WORLD_H
#define FB_GAME_HANDLER_MAP_WORLD_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class map_world : public fb::handler::protocol<fb::game::server, game_reqs::map_world<V>>
{
public:
    map_world(fb::game::server& server);
    map_world(const map_world&)             = delete;
    map_world(map_world&&)                  = delete;
    map_world& operator= (const map_world&) = delete;
    map_world& operator= (map_world&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::map_world<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_MAP_WORLD_H
