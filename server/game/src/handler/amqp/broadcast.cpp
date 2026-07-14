#include <fb/game/handler/amqp/broadcast.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

broadcast::broadcast(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::Broadcast>(server)
{ }

async::task<void> broadcast::handle(const internal_resp::Broadcast& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.characters.on_broadcast(message);
}
