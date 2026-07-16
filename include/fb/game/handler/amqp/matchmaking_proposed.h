#ifndef FB_GAME_HANDLER_AMQP_MATCHMAKING_PROPOSED_H
#define FB_GAME_HANDLER_AMQP_MATCHMAKING_PROPOSED_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class matchmaking_proposed : public fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::Proposed>
{
public:
    matchmaking_proposed(fb::game::server& server);
    matchmaking_proposed(const matchmaking_proposed&)             = delete;
    matchmaking_proposed(matchmaking_proposed&&)                  = delete;
    matchmaking_proposed& operator= (const matchmaking_proposed&) = delete;
    matchmaking_proposed& operator= (matchmaking_proposed&&)      = delete;

public:
    async::task<void> handle(const fb::protocol::matchmaking::mq::Proposed& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_MATCHMAKING_PROPOSED_H
