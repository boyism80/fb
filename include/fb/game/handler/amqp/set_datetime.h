#ifndef FB_GAME_HANDLER_AMQP_SET_DATETIME_H
#define FB_GAME_HANDLER_AMQP_SET_DATETIME_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class set_datetime : public fb::handler::amqp<fb::game::server, internal_resp::SetDateTime>
{
public:
    set_datetime(fb::game::server& server);
    set_datetime(const set_datetime&)             = delete;
    set_datetime(set_datetime&&)                  = delete;
    set_datetime& operator= (const set_datetime&) = delete;
    set_datetime& operator= (set_datetime&&)      = delete;

public:
    async::task<void> handle(const internal_resp::SetDateTime& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_SET_DATETIME_H
