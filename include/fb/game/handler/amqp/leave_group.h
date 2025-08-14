#ifndef FB_GAME_HANDLER_AMQP_LEAVE_GROUP_H
#define FB_GAME_HANDLER_AMQP_LEAVE_GROUP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class leave_group : public fb::amqp_handler<fb::game::server, internal_resp::LeaveGroup>
{
public:
    leave_group(fb::game::server& server);
    leave_group(const leave_group&)             = delete;
    leave_group(leave_group&&)                  = delete;
    leave_group& operator= (const leave_group&) = delete;
    leave_group& operator= (leave_group&&)      = delete;

public:
    async::task<void> handle(const internal_resp::LeaveGroup& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_LEAVE_GROUP_H
