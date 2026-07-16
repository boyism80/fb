#include <fb/game/handler/amqp/deliver_system_storage.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

deliver_system_storage::deliver_system_storage(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::DeliverSystemStorage>(server)
{ }

async::task<void> deliver_system_storage::handle(const internal_resp::DeliverSystemStorage& message)
{
    if (message.error != 0)
        co_return;

    this->server.system_storage.apply_deliver_entries(message.entries);
    co_return;
}
