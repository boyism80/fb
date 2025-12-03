#ifndef FB_GAME_HANDLER_AMQP_CREATE_CLAN_H
#define FB_GAME_HANDLER_AMQP_CREATE_CLAN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class create_clan : public fb::handler::amqp<fb::game::server, internal_resp::ClanDetails>
{
public:
    create_clan(fb::game::server& server);
    create_clan(const create_clan&)             = delete;
    create_clan(create_clan&&)                  = delete;
    create_clan& operator= (const create_clan&) = delete;
    create_clan& operator= (create_clan&&)      = delete;

public:
    async::task<void> handle(const internal_resp::ClanDetails& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_CREATE_CLAN_H

