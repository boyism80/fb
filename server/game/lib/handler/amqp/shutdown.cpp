#include <fb/game/handler/amqp/shutdown.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

shutdown::shutdown(fb::game::server& server) :
    fb::amqp_handler<fb::game::server, internal_resp::Shutdown>(server)
{ }

async::task<void> shutdown::handle(const internal_resp::Shutdown& message)
{
    this->server.exit();
    co_return;
}
