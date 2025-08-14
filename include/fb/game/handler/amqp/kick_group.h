#ifndef FB_GAME_HANDLER_AMQP_KICK_GROUP_H
#define FB_GAME_HANDLER_AMQP_KICK_GROUP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class kick_group : public fb::amqp_handler<fb::game::server, internal_resp::KickGroup>
{
public:
    kick_group(fb::game::server& server);
    kick_group(const kick_group&)             = delete;
    kick_group(kick_group&&)                  = delete;
    kick_group& operator= (const kick_group&) = delete;
    kick_group& operator= (kick_group&&)      = delete;

public:
    async::task<void> handle(const internal_resp::KickGroup& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_KICK_GROUP_H
