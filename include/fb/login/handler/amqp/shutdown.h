#ifndef FB_LOGIN_HANDLER_AMQP_SHUTDOWN_H
#define FB_LOGIN_HANDLER_AMQP_SHUTDOWN_H

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::amqp {

class shutdown : public fb::amqp_handler<fb::login::server, internal_resp::Shutdown>
{
public:
    shutdown(fb::login::server& server);
    shutdown(const shutdown&)             = delete;
    shutdown(shutdown&&)                  = delete;
    shutdown& operator= (const shutdown&) = delete;
    shutdown& operator= (shutdown&&)      = delete;

public:
    async::task<void> handle(const internal_resp::Shutdown& message) override;
};

} // namespace fb::login::handler::amqp

#endif // FB_LOGIN_HANDLER_AMQP_SHUTDOWN_H
