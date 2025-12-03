#ifndef FB_GAME_HANDLER_AMQP_CREATE_GROUP_H
#define FB_GAME_HANDLER_AMQP_CREATE_GROUP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class create_group : public fb::handler::amqp<fb::game::server, internal_resp::GroupDetails>
{
public:
    create_group(fb::game::server& server);
    create_group(const create_group&)             = delete;
    create_group(create_group&&)                  = delete;
    create_group& operator= (const create_group&) = delete;
    create_group& operator= (create_group&&)      = delete;

public:
    async::task<void> handle(const internal_resp::GroupDetails& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_CREATE_GROUP_H

