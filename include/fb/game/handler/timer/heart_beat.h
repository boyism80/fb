#ifndef FB_GAME_HANDLER_TIMER_HEART_BEAT_H
#define FB_GAME_HANDLER_TIMER_HEART_BEAT_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::timer {

class heart_beat : public fb::handler::timer<fb::game::server>
{
public:
    heart_beat(fb::game::server& server);

    async::task<void> handle() override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_HEART_BEAT_H
