#include <fb/game/handler/amqp/friend_relation.h>
#include <fb/game/character.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

friend_relation::friend_relation(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::FriendRelation>(server)
{ }

async::task<void> friend_relation::handle(const internal_resp::FriendRelation& message)
{
    auto ch = this->server.characters.find(message.user);
    if (ch == nullptr)
        co_return;

    auto weak   = ch->weak_from_this_as<fb::game::character>();
    auto before = this->server.threads.current();
    co_await this->server.threads.switching(weak);

    ch = weak.lock();
    if (ch != nullptr)
        ch->update_friend_relation(message.friend_uid, message.friend_name, message.mutual);

    if (before != nullptr)
        co_await before->switching();
}
