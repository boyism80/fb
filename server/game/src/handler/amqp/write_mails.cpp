#include <fb/game/handler/amqp/write_mails.h>
#include <fb/game/server.h>
#include <fb/game/mail_box.h>

using namespace fb::game::handler::amqp;

write_mails::write_mails(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::WriteMails>(server)
{ }

async::task<void> write_mails::handle(const internal_resp::WriteMails& message)
{
    if (message.error != 0)
        co_return;

    auto snapshots     = std::vector<mail_box::summary>{};
    auto user_ids      = std::vector<uint32_t>{};
    auto unread_counts = std::vector<uint16_t>{};
    snapshots.reserve(message.entries.size());
    user_ids.reserve(message.entries.size());
    unread_counts.reserve(message.entries.size());

    for (const auto& entry : message.entries)
    {
        const auto& m       = entry.mail;
        const auto  user_id = m.user != 0 ? m.user : entry.user;
        user_ids.push_back(user_id);
        unread_counts.push_back(entry.unread);
        snapshots.push_back(mail_box::summary{m.id, m.user, m.sender, m.read, m.title, m.created_date});
    }

    co_await this->server.mail.on_received_batch(snapshots, user_ids, unread_counts);
}
