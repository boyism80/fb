#ifndef FB_GAME_HANDLER_AMQP_BROADCAST_SAVE_H
#define FB_GAME_HANDLER_AMQP_BROADCAST_SAVE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class broadcast_save : public fb::handler::amqp<fb::game::server, internal_resp::BroadcastSave>
{
public:
    broadcast_save(fb::game::server& server);
    broadcast_save(const broadcast_save&)             = delete;
    broadcast_save(broadcast_save&&)                  = delete;
    broadcast_save& operator= (const broadcast_save&) = delete;
    broadcast_save& operator= (broadcast_save&&)      = delete;

public:
    async::task<void> handle(const internal_resp::BroadcastSave& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_BROADCAST_SAVE_H
