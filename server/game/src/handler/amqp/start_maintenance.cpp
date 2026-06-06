#include <fb/game/handler/amqp/start_maintenance.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

start_maintenance::start_maintenance(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::StartMaintenance>(server)
{ }

async::task<void> start_maintenance::handle(const internal_resp::StartMaintenance& message)
{
    auto guard = this->server.characters.enter_write();
    guard.value().on_start_maintenance(message);
    co_return;
}
