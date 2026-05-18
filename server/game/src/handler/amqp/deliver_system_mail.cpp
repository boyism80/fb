#include <fb/game/handler/amqp/deliver_system_mail.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

deliver_system_mail::deliver_system_mail(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::DeliverSystemMail>(server)
{ }

async::task<void> deliver_system_mail::handle(const internal_resp::DeliverSystemMail& message)
{
    if (message.error != 0)
        co_return;

    co_await this->server.on_mail_write_entries(message.entries);
}
