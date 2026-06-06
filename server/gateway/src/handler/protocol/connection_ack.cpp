#include <fb/gateway/handler/protocol/connection_ack.h>

using namespace fb::gateway::handler::protocol;

connection_ack::connection_ack(fb::gateway::server& server) :
    fb::handler::protocol<fb::gateway::server, gateway_reqs::connection_ack>(server)
{ }

async::task<bool> connection_ack::handle(fb::socket<fb::gateway::session>& session,
                                         gateway_reqs::connection_ack&     request)
{
    co_return true;
}
