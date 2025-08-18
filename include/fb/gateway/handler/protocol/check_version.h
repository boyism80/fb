#ifndef __FB_GATEWAY_HANDLER_CHECK_VERSION_H__
#define __FB_GATEWAY_HANDLER_CHECK_VERSION_H__

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::protocol {

class check_version : public fb::handler::protocol<fb::gateway::server, fb::protocol::gateway::request::version>
{
public:
    check_version(fb::gateway::server& server);
    check_version(const check_version&)             = delete;
    check_version(check_version&&)                  = delete;
    check_version& operator= (const check_version&) = delete;
    check_version& operator= (check_version&&)      = delete;

    async::task<bool> handle(fb::socket<fb::gateway::session>& session, fb::protocol::gateway::request::version& request) override;
};

} // namespace fb::gateway::handler::protocol

#endif // __FB_GATEWAY_HANDLER_CHECK_VERSION_H__
