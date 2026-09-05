#ifndef FB_GAME_HANDLER_AMQP_FRIEND_MESSAGE_H
#define FB_GAME_HANDLER_AMQP_FRIEND_MESSAGE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class friend_message : public fb::handler::amqp<fb::game::server, internal_resp::FriendMessage>
{
public:
    friend_message(fb::game::server& server);
    friend_message(const friend_message&)             = delete;
    friend_message(friend_message&&)                  = delete;
    friend_message& operator= (const friend_message&) = delete;
    friend_message& operator= (friend_message&&)      = delete;

public:
    async::task<void> handle(const internal_resp::FriendMessage& message) override;
};

} // namespace fb::game::handler::amqp

#endif
