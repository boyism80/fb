#ifndef FB_GAME_HANDLER_TIMER_ANNOUNCE_H
#define FB_GAME_HANDLER_TIMER_ANNOUNCE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::timer {

class announce : public fb::handler::timer<fb::game::server>
{
public:
    announce(fb::game::server& server);

    async::task<void> handle() override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_ANNOUNCE_H
