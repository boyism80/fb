#ifndef FB_GAME_HANDLER_AMQP_UPDATED_CLAN_H
#define FB_GAME_HANDLER_AMQP_UPDATED_CLAN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class updated_clan : public fb::handler::amqp<fb::game::server, internal_resp::UpdatedClan>
{
public:
    updated_clan(fb::game::server& server);
    updated_clan(const updated_clan&)             = delete;
    updated_clan(updated_clan&&)                  = delete;
    updated_clan& operator= (const updated_clan&) = delete;
    updated_clan& operator= (updated_clan&&)      = delete;

public:
    async::task<void> handle(const internal_resp::UpdatedClan& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_UPDATED_CLAN_H
