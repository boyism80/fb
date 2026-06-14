#include <fb/game/handler/amqp/ban.h>
#include <fb/game/character.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;
using namespace std::chrono_literals;

ban::ban(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::Ban>(server)
{ }

async::task<void> ban::handle(const internal_resp::Ban& message)
{
    std::weak_ptr<fb::game::character> weak;
    {
        auto guard = co_await this->server.characters.enter_write_async();
        auto ch    = guard.value().find(message.name);
        if (ch == nullptr)
            co_return;

        weak = ch->weak_from_this_as<fb::game::character>();
    }

    co_await this->server.threads.switching(weak);

    auto ch = weak.lock();
    if (ch == nullptr)
        co_return;

    ch->message(fb::game::character::container::build_ban_message(message.reason, message.expire_date),
                fb::game::MESSAGE_TYPE::POPUP);

    co_await ch->thread()->sleep(1s);

    ch = weak.lock();
    if (ch == nullptr)
        co_return;

    if (auto socket_ptr = ch->socket_ptr(); socket_ptr != nullptr)
        socket_ptr->close();
}
