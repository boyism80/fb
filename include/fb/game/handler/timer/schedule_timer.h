#ifndef FB_GAME_HANDLER_TIMER_SCHEDULE_TIMER_H
#define FB_GAME_HANDLER_TIMER_SCHEDULE_TIMER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::timer {

class schedule_timer : public fb::handler::timer<fb::game::server>
{
public:
    schedule_timer(fb::game::server& server);

    async::task<void> handle() override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_SCHEDULE_TIMER_H
