#include <fb/game/handler/amqp/start_maintenance.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

start_maintenance::start_maintenance(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::StartMaintenance>(server)
{ }

async::task<void> start_maintenance::handle(const internal_resp::StartMaintenance& message)
{
    this->server.characters.write([&message](auto& container) {
        container.foreach_enqueue([message = message.message](auto& ch) -> async::task<void> {
            if (ch->role() >= ROLE::ADMIN)
                co_return;

            auto socket_ptr = ch->socket_ptr();
            if (socket_ptr == nullptr)
                co_return;

            ch->message(message, MESSAGE_TYPE::STATE);
            socket_ptr->close();
        });
    });

    co_return;
}
