#ifndef FB_GAME_HANDLER_AMQP_BROADCAST_GROUP_H
#define FB_GAME_HANDLER_AMQP_BROADCAST_GROUP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class broadcast_group : public fb::handler::amqp<fb::game::server, internal_resp::BroadcastGroup>
{
public:
    broadcast_group(fb::game::server& server);
    broadcast_group(const broadcast_group&)             = delete;
    broadcast_group(broadcast_group&&)                  = delete;
    broadcast_group& operator= (const broadcast_group&) = delete;
    broadcast_group& operator= (broadcast_group&&)      = delete;

public:
    async::task<void> handle(const internal_resp::BroadcastGroup& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_BROADCAST_GROUP_H
