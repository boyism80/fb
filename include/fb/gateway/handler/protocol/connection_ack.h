#ifndef __FB_GATEWAY_HANDLER_CONNECTION_ACK_H__
#define __FB_GATEWAY_HANDLER_CONNECTION_ACK_H__

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::protocol {

namespace gateway_reqs = fb::protocol::gateway::request;

template <fb::protocol::CLIENT_VERSION V>
class connection_ack : public fb::handler::protocol<fb::gateway::server, gateway_reqs::connection_ack<V>>
{
public:
    connection_ack(fb::gateway::server& server);
    connection_ack(const connection_ack&)             = delete;
    connection_ack(connection_ack&&)                  = delete;
    connection_ack& operator= (const connection_ack&) = delete;
    connection_ack& operator= (connection_ack&&)      = delete;

    async::task<bool> handle(fb::socket<fb::gateway::session>& session,
                             gateway_reqs::connection_ack<V>&  request) override;
};

} // namespace fb::gateway::handler::protocol

#endif
