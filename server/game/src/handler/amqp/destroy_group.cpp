#include <fb/game/handler/amqp/destroy_group.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

destroy_group::destroy_group(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::DestroyGroup>(server)
{ }

async::task<void> destroy_group::handle(const internal_resp::DestroyGroup& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.group.on_error(message.error, message.actor.name);
    co_await this->server.group.on_destroyed(message.actor.name, message.group_id);
}
