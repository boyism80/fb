#ifndef __FB_LOGIN_HANDLER_META_DAT_H__
#define __FB_LOGIN_HANDLER_META_DAT_H__

#include <fb/handler.h>
#include <fb/login/server.h>
#include <fb/login/protocol/meta_dat.h>

namespace fb::login::handler::protocol {

namespace login_reqs = fb::protocol::login::request;

template <fb::protocol::CLIENT_VERSION V>
class meta_dat : public fb::handler::protocol<fb::login::server, login_reqs::meta_dat<V>>
{
public:
    meta_dat(fb::login::server& server);
    meta_dat(const meta_dat&)             = delete;
    meta_dat(meta_dat&&)                  = delete;
    meta_dat& operator= (const meta_dat&) = delete;
    meta_dat& operator= (meta_dat&&)      = delete;

    async::task<bool> handle(fb::socket<fb::login::session>& session, login_reqs::meta_dat<V>& request) override;
};

} // namespace fb::login::handler::protocol

#endif // __FB_LOGIN_HANDLER_META_DAT_H__
