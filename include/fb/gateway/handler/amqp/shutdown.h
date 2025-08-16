#ifndef FB_GATEWAY_HANDLER_AMQP_SHUTDOWN_H
#define FB_GATEWAY_HANDLER_AMQP_SHUTDOWN_H

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::amqp {

class shutdown : public fb::handler::amqp<fb::gateway::server, internal_resp::Shutdown>
{
public:
    shutdown(fb::gateway::server& server);
    shutdown(const shutdown&)             = delete;
    shutdown(shutdown&&)                  = delete;
    shutdown& operator= (const shutdown&) = delete;
    shutdown& operator= (shutdown&&)      = delete;

public:
    async::task<void> handle(const internal_resp::Shutdown& message) override;
};

} // namespace fb::gateway::handler::amqp

#endif // FB_GATEWAY_HANDLER_AMQP_SHUTDOWN_H
