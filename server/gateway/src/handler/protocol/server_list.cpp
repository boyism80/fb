#include <fb/gateway/handler/protocol/server_list.h>
#include <fb/gateway/server.h>

using namespace fb::gateway::handler::protocol;

template <fb::protocol::CLIENT_VERSION V>
server_list<V>::server_list(fb::gateway::server& server) :
    fb::handler::protocol<fb::gateway::server, gateway_reqs::server_list<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> server_list<V>::handle(fb::socket<fb::gateway::session>& session,
                                         gateway_reqs::server_list<V>&     request)
{
    switch (request.action)
    {
    case 0x00:
    {
        const auto& entrypoints = this->server.entrypoints();
        if (request.index >= entrypoints.size())
            co_return false;

        const auto& entry = entrypoints[request.index];
        std::ignore = this->server.transfer(session, entry.ip, entry.port, fb::protocol::internal::Service::Gateway);
        co_return true;
    }

    case 0x01:
    {
        this->server.send(session, this->server.endpoint_bytes());
        co_return true;
    }

    default:
        co_return false;
    }
}

template class server_list<fb::protocol::CLIENT_VERSION::v550>;
template class server_list<fb::protocol::CLIENT_VERSION::v565>;
template class server_list<fb::protocol::CLIENT_VERSION::v651>;
