#ifndef FB_GAME_HANDLER_MAP_WORLD_H
#define FB_GAME_HANDLER_MAP_WORLD_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class map_world : public fb::handler<fb::game::server, fb::protocol::game::request::map_world>
{
public:
    map_world(fb::game::server& server);
    map_world(const map_world&)             = delete;
    map_world(map_world&&)                  = delete;
    map_world& operator= (const map_world&) = delete;
    map_world& operator= (map_world&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::map_world& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_MAP_WORLD_H
