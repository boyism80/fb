#ifndef FB_GAME_HANDLER_AMQP_MATCHMAKING_TICKET_REMOVED_H
#define FB_GAME_HANDLER_AMQP_MATCHMAKING_TICKET_REMOVED_H

#include <fb/game/server.h>
#include <fb/handler.h>

namespace fb::game::handler::amqp {

class matchmaking_ticket_removed
    : public fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::TicketRemoved>
{
public:
    matchmaking_ticket_removed(fb::game::server& server);
    matchmaking_ticket_removed(const matchmaking_ticket_removed&)             = delete;
    matchmaking_ticket_removed(matchmaking_ticket_removed&&)                  = delete;
    matchmaking_ticket_removed& operator= (const matchmaking_ticket_removed&) = delete;
    matchmaking_ticket_removed& operator= (matchmaking_ticket_removed&&)      = delete;

public:
    async::task<void> handle(const fb::protocol::matchmaking::mq::TicketRemoved& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_MATCHMAKING_TICKET_REMOVED_H
