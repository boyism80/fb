#ifndef __FB_LOGIN_HANDLER_CREATE_ACCOUNT_H__
#define __FB_LOGIN_HANDLER_CREATE_ACCOUNT_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::protocol {

namespace login_reqs = fb::protocol::login::request;

template <fb::protocol::CLIENT_VERSION V>
class create_account : public fb::handler::protocol<fb::login::server, login_reqs::create<V>>
{
public:
    create_account(fb::login::server& server);
    create_account(const create_account&)             = delete;
    create_account(create_account&&)                  = delete;
    create_account& operator= (const create_account&) = delete;
    create_account& operator= (create_account&&)      = delete;

    async::task<bool> handle(fb::socket<fb::login::session>& session, login_reqs::create<V>& request) override;
};

} // namespace fb::login::handler::protocol

#endif // __FB_LOGIN_HANDLER_CREATE_ACCOUNT_H__
