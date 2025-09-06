#ifndef __FB_LOGIN_HANDLER_LOGIN_H__
#define __FB_LOGIN_HANDLER_LOGIN_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::protocol {

class login : public fb::handler::protocol<fb::login::server, fb::protocol::login::request::login>
{
public:
    login(fb::login::server& server);
    login(const login&)             = delete;
    login(login&&)                  = delete;
    login& operator= (const login&) = delete;
    login& operator= (login&&)      = delete;

    async::task<bool> handle(fb::socket<fb::login::session>& session, fb::protocol::login::request::login& request) override;
};

} // namespace fb::login::handler::protocol

#endif // __FB_LOGIN_HANDLER_LOGIN_H__
