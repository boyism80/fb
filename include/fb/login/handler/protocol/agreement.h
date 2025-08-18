#ifndef __FB_LOGIN_HANDLER_AGREEMENT_H__
#define __FB_LOGIN_HANDLER_AGREEMENT_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::protocol {

class agreement : public fb::handler::protocol<fb::login::server, fb::protocol::login::request::agreement>
{
public:
    agreement(fb::login::server& server);
    agreement(const agreement&)             = delete;
    agreement(agreement&&)                  = delete;
    agreement& operator= (const agreement&) = delete;
    agreement& operator= (agreement&&)      = delete;

    async::task<bool> handle(fb::socket<fb::login::session>& session, fb::protocol::login::request::agreement& request) override;
};

} // namespace fb::login::handler::protocol

#endif // __FB_LOGIN_HANDLER_AGREEMENT_H__
