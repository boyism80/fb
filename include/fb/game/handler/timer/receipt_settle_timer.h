#ifndef FB_GAME_HANDLER_TIMER_RECEIPT_SETTLE_TIMER_H
#define FB_GAME_HANDLER_TIMER_RECEIPT_SETTLE_TIMER_H

#include <fb/game/server.h>
#include <fb/handler.h>

#include <thread>

namespace fb::game::handler::timer {

class receipt_settle_timer : public fb::handler::timer<fb::game::server>
{
public:
    receipt_settle_timer(fb::game::server& server);

    async::task<void> handle(const fb::model::datetime& now, std::thread::id id) override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_RECEIPT_SETTLE_TIMER_H
