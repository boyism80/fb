#ifndef FB_GAME_HANDLER_AMQP_DESTROY_CLAN_H
#define FB_GAME_HANDLER_AMQP_DESTROY_CLAN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class destroy_clan : public fb::handler::amqp<fb::game::server, internal_resp::DestroyClan>
{
public:
    destroy_clan(fb::game::server& server);
    destroy_clan(const destroy_clan&)             = delete;
    destroy_clan(destroy_clan&&)                  = delete;
    destroy_clan& operator= (const destroy_clan&) = delete;
    destroy_clan& operator= (destroy_clan&&)      = delete;

public:
    async::task<void> handle(const internal_resp::DestroyClan& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_DESTROY_CLAN_H
