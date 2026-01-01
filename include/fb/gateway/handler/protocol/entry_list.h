#ifndef __FB_GATEWAY_HANDLER_ENTRY_LIST_H__
#define __FB_GATEWAY_HANDLER_ENTRY_LIST_H__

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::protocol {

namespace gateway_reqs = fb::protocol::gateway::request;

class entry_list : public fb::handler::protocol<fb::gateway::server, gateway_reqs::endpoint>
{
public:
    entry_list(fb::gateway::server& server);
    entry_list(const entry_list&)             = delete;
    entry_list(entry_list&&)                  = delete;
    entry_list& operator= (const entry_list&) = delete;
    entry_list& operator= (entry_list&&)      = delete;

    async::task<bool> handle(fb::socket<fb::gateway::session>& session, gateway_reqs::endpoint& request) override;
};

} // namespace fb::gateway::handler::protocol

#endif // __FB_GATEWAY_HANDLER_ENTRY_LIST_H__
