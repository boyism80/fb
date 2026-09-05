#ifndef FB_GAME_HANDLER_AMQP_FRIEND_RELATION_H
#define FB_GAME_HANDLER_AMQP_FRIEND_RELATION_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class friend_relation : public fb::handler::amqp<fb::game::server, internal_resp::FriendRelation>
{
public:
    friend_relation(fb::game::server& server);
    friend_relation(const friend_relation&)             = delete;
    friend_relation(friend_relation&&)                  = delete;
    friend_relation& operator= (const friend_relation&) = delete;
    friend_relation& operator= (friend_relation&&)      = delete;

public:
    async::task<void> handle(const internal_resp::FriendRelation& message) override;
};

} // namespace fb::game::handler::amqp

#endif
