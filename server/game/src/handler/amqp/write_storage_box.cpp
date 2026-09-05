#include <fb/game/handler/amqp/write_storage_box.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

write_storage_box::write_storage_box(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::WriteStorageBox>(server)
{ }

async::task<void> write_storage_box::handle(const internal_resp::WriteStorageBox& message)
{
    if (message.error != 0)
        co_return;

    this->server.system_storage.on_write_box(message.box);
    co_return;
}
