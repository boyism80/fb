#ifndef FB_GAME_HANDLER_DOOR_H
#define FB_GAME_HANDLER_DOOR_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class door : public fb::handler<fb::game::server, fb::protocol::game::request::door>
{
public:
    door(fb::game::server& server);
    door(const door&)             = delete;
    door(door&&)                  = delete;
    door& operator= (const door&) = delete;
    door& operator= (door&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::door& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_DOOR_H
