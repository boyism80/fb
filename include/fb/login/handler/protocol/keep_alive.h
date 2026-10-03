#ifndef __FB_LOGIN_HANDLER_KEEP_ALIVE_H__
#define __FB_LOGIN_HANDLER_KEEP_ALIVE_H__

#include <fb/handler.h>
#include <fb/login/protocol/keep_alive.h>
#include <fb/login/server.h>

namespace fb::login::handler::protocol {

namespace login_reqs = fb::protocol::login::request;

template <fb::protocol::CLIENT_VERSION V>
class keep_alive : public fb::handler::protocol<fb::login::server, login_reqs::keep_alive<V>>
{
public:
    keep_alive(fb::login::server& server);
    keep_alive(const keep_alive&)             = delete;
    keep_alive(keep_alive&&)                  = delete;
    keep_alive& operator= (const keep_alive&) = delete;
    keep_alive& operator= (keep_alive&&)      = delete;

    async::task<bool> handle(fb::socket<fb::login::session>& session, login_reqs::keep_alive<V>& request) override;
};

} // namespace fb::login::handler::protocol

#endif // __FB_LOGIN_HANDLER_KEEP_ALIVE_H__
