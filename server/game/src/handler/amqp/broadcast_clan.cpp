#include <fb/game/handler/amqp/broadcast_clan.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

broadcast_clan::broadcast_clan(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::BroadcastClan>(server)
{ }

async::task<void> broadcast_clan::handle(const internal_resp::BroadcastClan& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_clan_broadcast(message);
}
