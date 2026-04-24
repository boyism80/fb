#ifndef FB_LOGIN_HANDLER_AMQP_SET_DATETIME_H
#define FB_LOGIN_HANDLER_AMQP_SET_DATETIME_H

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::amqp {

class set_datetime : public fb::handler::amqp<fb::login::server, internal_resp::SetDateTime>
{
public:
    set_datetime(fb::login::server& server);
    set_datetime(const set_datetime&)             = delete;
    set_datetime(set_datetime&&)                  = delete;
    set_datetime& operator= (const set_datetime&) = delete;
    set_datetime& operator= (set_datetime&&)      = delete;

public:
    async::task<void> handle(const internal_resp::SetDateTime& message) override;
};

} // namespace fb::login::handler::amqp

#endif // FB_LOGIN_HANDLER_AMQP_SET_DATETIME_H
