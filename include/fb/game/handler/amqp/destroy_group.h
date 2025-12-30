#ifndef FB_GAME_HANDLER_AMQP_DESTROY_GROUP_H
#define FB_GAME_HANDLER_AMQP_DESTROY_GROUP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class destroy_group : public fb::handler::amqp<fb::game::server, internal_resp::DestroyGroup>
{
public:
    destroy_group(fb::game::server& server);
    destroy_group(const destroy_group&)              = delete;
    destroy_group(destroy_group&&)                   = delete;
    destroy_group& operator= (const destroy_group&)  = delete;
    destroy_group& operator= (const destroy_group&&) = delete;

public:
    async::task<void> handle(const internal_resp::DestroyGroup& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_DESTROY_GROUP_H
