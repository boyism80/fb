#include <fb/gateway/handler/amqp/shutdown.h>
#include <fb/gateway/server.h>

using namespace fb::gateway::handler::amqp;

shutdown::shutdown(fb::gateway::server& server) :
    fb::handler::amqp<fb::gateway::server, internal_resp::Shutdown>(server)
{ }

async::task<void> shutdown::handle(const internal_resp::Shutdown& message)
{
    this->server.exit();
    co_return;
}
