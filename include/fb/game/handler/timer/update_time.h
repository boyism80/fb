#ifndef FB_GAME_HANDLER_TIMER_UPDATE_TIME_H
#define FB_GAME_HANDLER_TIMER_UPDATE_TIME_H

#include <fb/game/server.h>
#include <fb/handler.h>

namespace fb::game::handler::timer {

class update_time : public fb::handler::timer<fb::game::server>
{
public:
    update_time(fb::game::server& server);

    async::task<void> handle() override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_UPDATE_TIME_H
