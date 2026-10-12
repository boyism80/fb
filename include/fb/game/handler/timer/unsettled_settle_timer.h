#ifndef FB_GAME_HANDLER_TIMER_UNSETTLED_SETTLE_TIMER_H
#define FB_GAME_HANDLER_TIMER_UNSETTLED_SETTLE_TIMER_H

#include <fb/game/server.h>
#include <fb/handler.h>

#include <thread>

namespace fb::game::handler::timer {

class unsettled_settle_timer : public fb::handler::timer<fb::game::server>
{
public:
    unsettled_settle_timer(fb::game::server& server);

    async::task<void> handle(const fb::model::datetime& now, std::thread::id id) override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_UNSETTLED_SETTLE_TIMER_H
