#include <fb/game/handler/amqp/start_maintenance.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

start_maintenance::start_maintenance(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::StartMaintenance>(server)
{ }

async::task<void> start_maintenance::handle(const internal_resp::StartMaintenance& message)
{
    // Collect all regular (non-admin) users first
    std::vector<std::shared_ptr<fb::game::character>> regular_users;

    this->server.characters.read([&regular_users](const auto& container) {
        for (auto& [_, ch] : container)
        {
            if (ch->role() < ROLE::ADMIN)
            {
                regular_users.push_back(ch);
            }
        }
    });

    // Disconnect regular users (outside of read lock)
    for (auto& ch : regular_users)
    {
        auto socket_ptr = ch->socket_ptr();
        if (socket_ptr != nullptr)
        {
            // Send maintenance message before disconnecting
            ch->message(message.message, MESSAGE_TYPE::STATE);
            socket_ptr->close();
        }
    }

    co_return;
}
