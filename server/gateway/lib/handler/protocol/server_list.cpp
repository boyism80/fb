#include <fb/gateway/handler/protocol/server_list.h>
#include <fb/gateway/server.h>

using namespace fb::gateway::handler::protocol;

server_list::server_list(fb::gateway::server& server) :
    fb::handler::protocol<fb::gateway::server, gateway_reqs::server_list>(server)
{ }

async::task<bool> server_list::handle(fb::socket<fb::gateway::session>& session, gateway_reqs::server_list& request)
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
