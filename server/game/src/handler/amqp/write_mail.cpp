#include <fb/game/handler/amqp/write_mail.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

write_mail::write_mail(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::WriteMail>(server)
{ }

async::task<void> write_mail::handle(const internal_resp::WriteMail& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    this->server.on_write_mail(message);
}
