#ifndef __FB_LOGIN_HANDLER_CHANGE_PASSWORD_H__
#define __FB_LOGIN_HANDLER_CHANGE_PASSWORD_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::protocol {

class change_password : public fb::handler::protocol<fb::login::server, fb::protocol::login::request::update_pw>
{
public:
    change_password(fb::login::server& server);
    change_password(const change_password&)             = delete;
    change_password(change_password&&)                  = delete;
    change_password& operator= (const change_password&) = delete;
    change_password& operator= (change_password&&)      = delete;

    async::task<bool> handle(fb::socket<fb::login::session>& session, fb::protocol::login::request::update_pw& request) override;
};

} // namespace fb::login::handler::protocol

#endif // __FB_LOGIN_HANDLER_CHANGE_PASSWORD_H__
