#include <fb/game/handler/amqp/updated_group.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

updated_group::updated_group(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::UpdatedGroup>(server)
{ }

async::task<void> updated_group::handle(const internal_resp::UpdatedGroup& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_updated_group(message);
}
