#ifndef FB_GAME_HANDLER_AMQP_JOIN_CLAN_H
#define FB_GAME_HANDLER_AMQP_JOIN_CLAN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class join_clan : public fb::handler::amqp<fb::game::server, internal_resp::JoinClan>
{
public:
    join_clan(fb::game::server& server);
    join_clan(const join_clan&)             = delete;
    join_clan(join_clan&&)                  = delete;
    join_clan& operator= (const join_clan&) = delete;
    join_clan& operator= (join_clan&&)      = delete;

public:
    async::task<void> handle(const internal_resp::JoinClan& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_JOIN_CLAN_H
