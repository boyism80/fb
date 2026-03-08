#ifndef __FB_GATEWAY_HANDLER_VERSION_H__
#define __FB_GATEWAY_HANDLER_VERSION_H__

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::protocol {

namespace gateway_reqs = fb::protocol::gateway::request;

class version : public fb::handler::protocol<fb::gateway::server, gateway_reqs::version>
{
public:
    version(fb::gateway::server& server);
    version(const version&)             = delete;
    version(version&&)                  = delete;
    version& operator= (const version&) = delete;
    version& operator= (version&&)      = delete;

    async::task<bool> handle(fb::socket<fb::gateway::session>& session, gateway_reqs::version& request) override;
};

} // namespace fb::gateway::handler::protocol

#endif
