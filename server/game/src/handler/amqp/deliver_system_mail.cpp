#include <fb/game/handler/amqp/deliver_system_mail.h>
#include <fb/game/server.h>
#include <fb/game/mail_box.h>

using namespace fb::game::handler::amqp;

deliver_system_mail::deliver_system_mail(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::DeliverSystemMail>(server)
{ }

async::task<void> deliver_system_mail::handle(const internal_resp::DeliverSystemMail& message)
{
    if (message.error != 0)
        co_return;

    auto entries = std::vector<mail_box::received>{};
    entries.reserve(message.entries.size());

    for (const auto& entry : message.entries)
    {
        const auto& m = entry.mail;
        entries.push_back(mail_box::received{
            .user_id        = m.user != 0 ? m.user : entry.user,
            .unread         = entry.unread,
            .snapshot       = mail_box::summary{m.id, m.user, m.sender, m.read, m.title, m.created_date},
            .system_mail_id = m.system_mail_id,
        });
    }

    co_await this->server.mail.on_received_batch(entries);
}
