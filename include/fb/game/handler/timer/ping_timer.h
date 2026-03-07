#ifndef FB_GAME_HANDLER_TIMER_PING_TIMER_H
#define FB_GAME_HANDLER_TIMER_PING_TIMER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::timer {

/**
 * Thread timer: every 1s, for each character in this thread, send ping if 5s elapsed
 * and pong was received (or first ping). Token is random.
 */
class ping_timer : public fb::handler::timer<fb::game::server>
{
public:
    ping_timer(fb::game::server& server);

    async::task<void> handle(const fb::model::datetime& now, std::thread::id id) override;
};

} // namespace fb::game::handler::timer

#endif
