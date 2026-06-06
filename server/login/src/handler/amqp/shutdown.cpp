#include <fb/login/handler/amqp/shutdown.h>
#include <fb/login/server.h>

using namespace fb::login::handler::amqp;

shutdown::shutdown(fb::login::server& server) :
    fb::handler::amqp<fb::login::server, internal_resp::Shutdown>(server)
{ }

async::task<void> shutdown::handle(const internal_resp::Shutdown& message)
{
    this->server.exit();
    co_return;
}
