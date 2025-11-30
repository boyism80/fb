#ifndef __FB_GAME_HANDLER_TIMER_LOG_FLUSH_H__
#define __FB_GAME_HANDLER_TIMER_LOG_FLUSH_H__

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::timer {

class log_flush : public fb::handler::timer<fb::game::server>
{
public:
    log_flush(fb::game::server& server);

    async::task<void> handle() override;
};

} // namespace fb::game::handler::timer

#endif // !__FB_GAME_HANDLER_TIMER_LOG_FLUSH_H__
