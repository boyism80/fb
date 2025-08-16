#ifndef FB_GAME_HANDLER_TIMER_MOB_RESPAWN_TIMER_H
#define FB_GAME_HANDLER_TIMER_MOB_RESPAWN_TIMER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::timer {

class mob_respawn_timer : public fb::handler::timer<fb::game::server>
{
public:
    mob_respawn_timer(fb::game::server& server);

    async::task<void> handle(const fb::model::datetime& now, std::thread::id id) override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_MOB_RESPAWN_TIMER_H
