#ifndef __FB_LOGIN_HANDLER_COMPLETE_H__
#define __FB_LOGIN_HANDLER_COMPLETE_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::protocol {

namespace login_reqs = fb::protocol::login::request;

template <fb::protocol::CLIENT_VERSION V>
class complete : public fb::handler::protocol<fb::login::server, login_reqs::complete<V>>
{
public:
    complete(fb::login::server& server);
    complete(const complete&)             = delete;
    complete(complete&&)                  = delete;
    complete& operator= (const complete&) = delete;
    complete& operator= (complete&&)      = delete;

    async::task<bool> handle(fb::socket<fb::login::session>& session, login_reqs::complete<V>& request) override;
};

} // namespace fb::login::handler::protocol

#endif // __FB_LOGIN_HANDLER_COMPLETE_H__
