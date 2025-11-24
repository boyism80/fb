#include <fb/game/handler/amqp/broadcast_save.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

broadcast_save::broadcast_save(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::BroadcastSave>(server)
{ }

async::task<void> broadcast_save::handle(const internal_resp::BroadcastSave& message)
{
    this->server.save();
    co_return;
}
