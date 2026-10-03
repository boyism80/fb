#ifndef FB_GAME_HANDLER_AMQP_MATCHMAKING_DISSOLVED_H
#define FB_GAME_HANDLER_AMQP_MATCHMAKING_DISSOLVED_H

#include <fb/game/server.h>
#include <fb/handler.h>

namespace fb::game::handler::amqp {

class matchmaking_dissolved : public fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::MatchDissolved>
{
public:
    matchmaking_dissolved(fb::game::server& server);
    matchmaking_dissolved(const matchmaking_dissolved&)             = delete;
    matchmaking_dissolved(matchmaking_dissolved&&)                  = delete;
    matchmaking_dissolved& operator= (const matchmaking_dissolved&) = delete;
    matchmaking_dissolved& operator= (matchmaking_dissolved&&)      = delete;

public:
    async::task<void> handle(const fb::protocol::matchmaking::mq::MatchDissolved& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_MATCHMAKING_DISSOLVED_H
