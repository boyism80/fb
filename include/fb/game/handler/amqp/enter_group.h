#ifndef FB_GAME_HANDLER_AMQP_ENTER_GROUP_H
#define FB_GAME_HANDLER_AMQP_ENTER_GROUP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class enter_group : public fb::handler::amqp<fb::game::server, internal_resp::EnterGroup>
{
public:
    enter_group(fb::game::server& server);
    enter_group(const enter_group&)             = delete;
    enter_group(enter_group&&)                  = delete;
    enter_group& operator= (const enter_group&) = delete;
    enter_group& operator= (enter_group&&)      = delete;

public:
    async::task<void> handle(const internal_resp::EnterGroup& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_ENTER_GROUP_H
