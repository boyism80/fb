#ifndef FB_GAME_HANDLER_DOOR_H
#define FB_GAME_HANDLER_DOOR_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class door : public fb::handler::protocol<fb::game::server, game_reqs::door<V>>
{
public:
    door(fb::game::server& server);
    door(const door&)             = delete;
    door(door&&)                  = delete;
    door& operator= (const door&) = delete;
    door& operator= (door&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::door<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_DOOR_H
