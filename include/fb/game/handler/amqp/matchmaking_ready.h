#ifndef FB_GAME_HANDLER_AMQP_MATCHMAKING_READY_H
#define FB_GAME_HANDLER_AMQP_MATCHMAKING_READY_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class matchmaking_ready : public fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::Ready>
{
public:
    matchmaking_ready(fb::game::server& server);
    matchmaking_ready(const matchmaking_ready&)             = delete;
    matchmaking_ready(matchmaking_ready&&)                  = delete;
    matchmaking_ready& operator= (const matchmaking_ready&) = delete;
    matchmaking_ready& operator= (matchmaking_ready&&)      = delete;

public:
    async::task<void> handle(const fb::protocol::matchmaking::mq::Ready& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_MATCHMAKING_READY_H
