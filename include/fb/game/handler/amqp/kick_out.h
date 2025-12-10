#ifndef FB_GAME_HANDLER_AMQP_KICK_OUT_H
#define FB_GAME_HANDLER_AMQP_KICK_OUT_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class kick_out : public fb::handler::amqp<fb::game::server, internal_resp::KickOut>
{
public:
    kick_out(fb::game::server& server);
    kick_out(const kick_out&)             = delete;
    kick_out(kick_out&&)                  = delete;
    kick_out& operator= (const kick_out&) = delete;
    kick_out& operator= (kick_out&&)      = delete;

public:
    async::task<void> handle(const internal_resp::KickOut& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_KICK_OUT_H
