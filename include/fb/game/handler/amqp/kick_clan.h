#ifndef FB_GAME_HANDLER_AMQP_KICK_CLAN_H
#define FB_GAME_HANDLER_AMQP_KICK_CLAN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class kick_clan : public fb::handler::amqp<fb::game::server, internal_resp::KickClan>
{
public:
    kick_clan(fb::game::server& server);
    kick_clan(const kick_clan&)             = delete;
    kick_clan(kick_clan&&)                  = delete;
    kick_clan& operator= (const kick_clan&) = delete;
    kick_clan& operator= (kick_clan&&)      = delete;

public:
    async::task<void> handle(const internal_resp::KickClan& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_KICK_CLAN_H
