#include <fb/gateway/handler/protocol/connection_ack.h>

using namespace fb::gateway::handler::protocol;

template <fb::protocol::CLIENT_VERSION V>
connection_ack<V>::connection_ack(fb::gateway::server& server) :
    fb::handler::protocol<fb::gateway::server, gateway_reqs::connection_ack<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> connection_ack<V>::handle(fb::socket<fb::gateway::session>& session,
                                            gateway_reqs::connection_ack<V>&  request)
{
    co_return true;
}

template class connection_ack<fb::protocol::CLIENT_VERSION::v550>;
template class connection_ack<fb::protocol::CLIENT_VERSION::v565>;
template class connection_ack<fb::protocol::CLIENT_VERSION::v651>;
