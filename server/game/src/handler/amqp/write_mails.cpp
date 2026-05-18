#include <fb/game/handler/amqp/write_mails.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

write_mails::write_mails(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::WriteMails>(server)
{ }

async::task<void> write_mails::handle(const internal_resp::WriteMails& message)
{
    if (message.error != 0)
        co_return;

    co_await this->server.on_mail_write_entries(message.entries);
}
