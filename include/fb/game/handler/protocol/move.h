#ifndef FB_GAME_HANDLER_MOVE_H
#define FB_GAME_HANDLER_MOVE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class move : public fb::handler::protocol<fb::game::server, game_reqs::move<V>>
{
public:
    move(fb::game::server& server);
    move(const move&)             = delete;
    move(move&&)                  = delete;
    move& operator= (const move&) = delete;
    move& operator= (move&&)      = delete;

    async::task<bool> handle(fb::socket<character>&      session,
                             DIRECTION                   direction,
                             const fb::model::point16_t& position,
                             uint8_t                     walk_queue_slot);
    async::task<bool> handle(fb::socket<character>& session, game_reqs::move<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_MOVE_H
