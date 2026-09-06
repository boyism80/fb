#include <fb/game/handler/amqp/broadcast_group.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

broadcast_group::broadcast_group(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::BroadcastGroup>(server)
{ }

async::task<void> broadcast_group::handle(const internal_resp::BroadcastGroup& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.groups.on_error(message.error, "");
    co_await this->server.groups.on_broadcast(message.group, message.message, message.type);
}
