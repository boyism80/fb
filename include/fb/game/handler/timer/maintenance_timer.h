#ifndef FB_GAME_HANDLER_TIMER_MAINTENANCE_TIMER_H
#define FB_GAME_HANDLER_TIMER_MAINTENANCE_TIMER_H

#include <fb/game/server.h>
#include <fb/handler.h>

namespace fb::game::handler::timer {

class maintenance_timer : public fb::handler::timer<fb::game::server>
{
public:
    explicit maintenance_timer(fb::game::server& server);

    async::task<void> handle() override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_MAINTENANCE_TIMER_H
