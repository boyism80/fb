#ifndef __FB_LOGIN_HANDLER_COMPLETE_H__
#define __FB_LOGIN_HANDLER_COMPLETE_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::protocol {

class complete : public fb::handler::protocol<fb::login::server, fb::protocol::login::request::complete>
{
public:
    complete(fb::login::server& server);
    complete(const complete&)             = delete;
    complete(complete&&)                  = delete;
    complete& operator= (const complete&) = delete;
    complete& operator= (complete&&)      = delete;

    async::task<bool> handle(fb::socket<fb::login::session>& session, fb::protocol::login::request::complete& request) override;
};

} // namespace fb::login::handler::protocol

#endif // __FB_LOGIN_HANDLER_COMPLETE_H__
