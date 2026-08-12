#include <fb/game/handler/amqp/friend_message.h>
#include <fb/game/character.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

friend_message::friend_message(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::FriendMessage>(server)
{ }

async::task<void> friend_message::handle(const internal_resp::FriendMessage& message)
{
    auto ch = this->server.characters.find(message.to_uid);
    if (ch == nullptr)
        co_return;

    auto weak   = ch->weak_from_this_as<fb::game::character>();
    auto before = this->server.threads.current();
    co_await this->server.threads.switching(weak);

    ch = weak.lock();
    if (ch != nullptr)
        ch->message(message.message, static_cast<fb::game::MESSAGE_TYPE>(message.type));

    if (before != nullptr)
        co_await before->switching();
}
