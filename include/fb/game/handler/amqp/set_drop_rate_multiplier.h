#ifndef FB_GAME_HANDLER_AMQP_SET_DROP_RATE_MULTIPLIER_H
#define FB_GAME_HANDLER_AMQP_SET_DROP_RATE_MULTIPLIER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class set_drop_rate_multiplier : public fb::handler::amqp<fb::game::server, internal_resp::SetDropRateMultiplier>
{
public:
    set_drop_rate_multiplier(fb::game::server& server);
    set_drop_rate_multiplier(const set_drop_rate_multiplier&)             = delete;
    set_drop_rate_multiplier(set_drop_rate_multiplier&&)                  = delete;
    set_drop_rate_multiplier& operator= (const set_drop_rate_multiplier&) = delete;
    set_drop_rate_multiplier& operator= (set_drop_rate_multiplier&&)      = delete;

public:
    async::task<void> handle(const internal_resp::SetDropRateMultiplier& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_SET_DROP_RATE_MULTIPLIER_H
