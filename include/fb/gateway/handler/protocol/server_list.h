#ifndef __FB_GATEWAY_HANDLER_SERVER_LIST_H__
#define __FB_GATEWAY_HANDLER_SERVER_LIST_H__

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::protocol {

namespace gateway_reqs = fb::protocol::gateway::request;

template <fb::protocol::CLIENT_VERSION V>
class server_list : public fb::handler::protocol<fb::gateway::server, gateway_reqs::server_list<V>>
{
public:
    server_list(fb::gateway::server& server);
    server_list(const server_list&)             = delete;
    server_list(server_list&&)                  = delete;
    server_list& operator= (const server_list&) = delete;
    server_list& operator= (server_list&&)      = delete;

    async::task<bool> handle(fb::socket<fb::gateway::session>& session, gateway_reqs::server_list<V>& request) override;
};

} // namespace fb::gateway::handler::protocol

#endif
