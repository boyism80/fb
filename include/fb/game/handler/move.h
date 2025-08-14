#ifndef FB_GAME_HANDLER_MOVE_H
#define FB_GAME_HANDLER_MOVE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class move : public fb::handler<fb::game::server, fb::protocol::game::request::move>
{
public:
    move(fb::game::server& server);
    move(const move&)             = delete;
    move(move&&)                  = delete;
    move& operator= (const move&) = delete;
    move& operator= (move&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, DIRECTION direction, const fb::model::point16_t& position);
    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::move& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_MOVE_H
