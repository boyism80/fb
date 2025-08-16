#ifndef FB_GAME_HANDLER_AMQP_BROADCAST_CLAN_H
#define FB_GAME_HANDLER_AMQP_BROADCAST_CLAN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class broadcast_clan : public fb::handler::amqp<fb::game::server, internal_resp::BroadcastClan>
{
public:
    broadcast_clan(fb::game::server& server);
    broadcast_clan(const broadcast_clan&)             = delete;
    broadcast_clan(broadcast_clan&&)                  = delete;
    broadcast_clan& operator= (const broadcast_clan&) = delete;
    broadcast_clan& operator= (broadcast_clan&&)      = delete;

public:
    async::task<void> handle(const internal_resp::BroadcastClan& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_BROADCAST_CLAN_H
