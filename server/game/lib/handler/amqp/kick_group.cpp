#include <fb/game/handler/amqp/kick_group.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

kick_group::kick_group(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::KickGroup>(server)
{ }

async::task<void> kick_group::handle(const internal_resp::KickGroup& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_kick_group(message);
}
