#ifndef FB_GAME_HANDLER_AMQP_UPDATED_GROUP_H
#define FB_GAME_HANDLER_AMQP_UPDATED_GROUP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class updated_group : public fb::handler::amqp<fb::game::server, internal_resp::UpdatedGroup>
{
public:
    updated_group(fb::game::server& server);
    updated_group(const updated_group&)              = delete;
    updated_group(updated_group&&)                   = delete;
    updated_group& operator= (const updated_group&)  = delete;
    updated_group& operator= (const updated_group&&) = delete;

public:
    async::task<void> handle(const internal_resp::UpdatedGroup& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_UPDATED_GROUP_H
