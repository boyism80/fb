#ifndef FB_GAME_HANDLER_AMQP_BROADCAST_H
#define FB_GAME_HANDLER_AMQP_BROADCAST_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class broadcast : public fb::handler::amqp<fb::game::server, internal_resp::Broadcast>
{
public:
    broadcast(fb::game::server& server);
    broadcast(const broadcast&)             = delete;
    broadcast(broadcast&&)                  = delete;
    broadcast& operator= (const broadcast&) = delete;
    broadcast& operator= (broadcast&&)      = delete;

public:
    async::task<void> handle(const internal_resp::Broadcast& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_BROADCAST_H
