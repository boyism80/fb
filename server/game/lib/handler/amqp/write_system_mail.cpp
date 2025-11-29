#include <fb/game/handler/amqp/write_system_mail.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

write_system_mail::write_system_mail(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::WriteSystemMail>(server)
{ }

async::task<void> write_system_mail::handle(const internal_resp::WriteSystemMail& message)
{
    // Immediately fetch system mails when notified of a new system mail
    co_await this->server.poll.system_mail.fetch();
    co_return;
}
