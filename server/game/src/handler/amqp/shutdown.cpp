#include <fb/game/handler/amqp/shutdown.h>

#include <fb/game/server.h>
#include <fb/logger.h>

#include <cstdlib>

using namespace fb::game::handler::amqp;

shutdown::shutdown(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::Shutdown>(server)
{ }

async::task<void> shutdown::handle(const internal_resp::Shutdown& message)
{
    fb::logger::warn("shutdown requested by AMQP fb.global");
    this->server.exit();
    co_return;
}
