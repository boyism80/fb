#ifndef FB_GAME_HANDLER_AMQP_SET_EXP_MULTIPLIER_H
#define FB_GAME_HANDLER_AMQP_SET_EXP_MULTIPLIER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class set_exp_multiplier : public fb::handler::amqp<fb::game::server, internal_resp::SetExpMultiplier>
{
public:
    set_exp_multiplier(fb::game::server& server);
    set_exp_multiplier(const set_exp_multiplier&)             = delete;
    set_exp_multiplier(set_exp_multiplier&&)                  = delete;
    set_exp_multiplier& operator= (const set_exp_multiplier&) = delete;
    set_exp_multiplier& operator= (set_exp_multiplier&&)      = delete;

public:
    async::task<void> handle(const internal_resp::SetExpMultiplier& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_SET_EXP_MULTIPLIER_H
