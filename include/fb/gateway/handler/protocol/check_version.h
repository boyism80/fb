#ifndef __FB_GATEWAY_HANDLER_CHECK_VERSION_H__
#define __FB_GATEWAY_HANDLER_CHECK_VERSION_H__

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::protocol {

namespace gateway_reqs = fb::protocol::gateway::request;

class check_version : public fb::handler::protocol<fb::gateway::server, gateway_reqs::version>
{
public:
    check_version(fb::gateway::server& server);
    check_version(const check_version&)             = delete;
    check_version(check_version&&)                  = delete;
    check_version& operator= (const check_version&) = delete;
    check_version& operator= (check_version&&)      = delete;

    async::task<bool> handle(fb::socket<fb::gateway::session>& session, gateway_reqs::version& request) override;
};

} // namespace fb::gateway::handler::protocol

#endif // __FB_GATEWAY_HANDLER_CHECK_VERSION_H__
