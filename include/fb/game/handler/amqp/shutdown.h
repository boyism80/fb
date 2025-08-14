#ifndef FB_GAME_HANDLER_AMQP_SHUTDOWN_H
#define FB_GAME_HANDLER_AMQP_SHUTDOWN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class shutdown : public fb::amqp_handler<fb::game::server, internal_resp::Shutdown>
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
