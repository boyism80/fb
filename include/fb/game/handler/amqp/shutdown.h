#ifndef FB_GAME_HANDLER_AMQP_SHUTDOWN_H
#define FB_GAME_HANDLER_AMQP_SHUTDOWN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class shutdown : public fb::handler::amqp<fb::game::server, internal_resp::Shutdown>
{
public:
    shutdown(fb::game::server& server);
    shutdown(const shutdown&)             = delete;
    shutdown(shutdown&&)                  = delete;
    shutdown& operator= (const shutdown&) = delete;
    shutdown& operator= (shutdown&&)      = delete;

public:
    async::task<void> handle(const internal_resp::Shutdown& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_SHUTDOWN_H
