#ifndef FB_GAME_HANDLER_DIRECTION_H
#define FB_GAME_HANDLER_DIRECTION_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class direction : public fb::handler<fb::game::server, fb::protocol::game::request::direction>
{
public:
    direction(fb::game::server& server);
    direction(const direction&)             = delete;
    direction(direction&&)                  = delete;
    direction& operator= (const direction&) = delete;
    direction& operator= (direction&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::direction& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_DIRECTION_H
