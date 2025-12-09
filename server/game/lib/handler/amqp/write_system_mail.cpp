#include <fb/game/handler/amqp/write_system_mail.h>
#include <fb/game/server.h>
#include <fb/game/system_mail.h>

using namespace fb::game::handler::amqp;

write_system_mail::write_system_mail(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::WriteSystemMail>(server)
{ }

async::task<void> write_system_mail::handle(const internal_resp::WriteSystemMail& message)
{
    if (message.error != 0)
        co_return;

    // Convert DTO to DAO
    auto mail = fb::game::system_mail(message.mail);

    // Append new system mail and notify all connected characters
    co_await this->server.system_mail.append(mail);
    co_return;
}
