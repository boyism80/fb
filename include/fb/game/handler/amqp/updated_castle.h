#ifndef FB_GAME_HANDLER_AMQP_UPDATED_CASTLE_H
#define FB_GAME_HANDLER_AMQP_UPDATED_CASTLE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class updated_castle : public fb::handler::amqp<fb::game::server, internal_resp::UpdatedCastle>
{
public:
    updated_castle(fb::game::server& server);
    updated_castle(const updated_castle&)             = delete;
    updated_castle(updated_castle&&)                  = delete;
    updated_castle& operator= (const updated_castle&) = delete;
    updated_castle& operator= (updated_castle&&)      = delete;

public:
    async::task<void> handle(const internal_resp::UpdatedCastle& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_UPDATED_CASTLE_H
