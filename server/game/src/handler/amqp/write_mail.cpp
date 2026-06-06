#include <fb/game/handler/amqp/write_mail.h>
#include <fb/game/server.h>
#include <fb/game/mail_box.h>

using namespace fb::game::handler::amqp;

write_mail::write_mail(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::WriteMail>(server)
{ }

async::task<void> write_mail::handle(const internal_resp::WriteMail& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    const auto& m        = message.mail;
    auto        snapshot = mail_box::summary{m.id, m.user, m.sender, m.read, m.title, m.created_date};
    co_await this->server.mail.on_received(m.user, message.unread, snapshot);
}
