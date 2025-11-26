#ifndef FB_GAME_HANDLER_TIMER_STORAGE_PENDING_TIMER_H
#define FB_GAME_HANDLER_TIMER_STORAGE_PENDING_TIMER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::timer
{

/**
 * @brief Periodically fetches storage pending payloads from internal server.
 */
class storage_pending_timer : public fb::handler::timer<fb::game::server>
{
public:
    explicit storage_pending_timer(fb::game::server& server);

    async::task<void> handle() override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_STORAGE_PENDING_TIMER_H

