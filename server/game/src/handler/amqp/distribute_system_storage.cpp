#include <fb/game/handler/amqp/distribute_system_storage.h>

#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

distribute_system_storage::distribute_system_storage(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::DistributeSystemStorage>(server)
{ }

async::task<void> distribute_system_storage::handle(const internal_resp::DistributeSystemStorage& message)
{
    if (message.error != 0)
        co_return;

    this->server.system_storage.on_distributed(message.entries);
    co_return;
}
