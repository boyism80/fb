#include <fb/gateway/handler/protocol/entry_list.h>
#include <fb/gateway/server.h>

using namespace fb::gateway::handler::protocol;

entry_list::entry_list(fb::gateway::server& server) :
    fb::handler::protocol<fb::gateway::server, gateway_reqs::endpoint>(server)
{ }

async::task<bool> entry_list::handle(fb::socket<fb::gateway::session>& session, gateway_reqs::endpoint& request)
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
