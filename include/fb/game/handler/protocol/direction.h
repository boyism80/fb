#ifndef FB_GAME_HANDLER_DIRECTION_H
#define FB_GAME_HANDLER_DIRECTION_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class direction : public fb::handler::protocol<fb::game::server, game_reqs::direction>
{
public:
    direction(fb::game::server& server);
    direction(const direction&)             = delete;
    direction(direction&&)                  = delete;
    direction& operator= (const direction&) = delete;
    direction& operator= (direction&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::direction& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_DIRECTION_H
