#ifndef FB_GAME_HANDLER_TIMER_STORAGE_PENDING_DISTRIBUTION_TIMER_H
#define FB_GAME_HANDLER_TIMER_STORAGE_PENDING_DISTRIBUTION_TIMER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::timer {

/**
 * @brief Thread timer that applies cached storage pending rewards to characters.
 */
class storage_pending_distribution_timer : public fb::handler::timer<fb::game::server>
{
public:
    explicit storage_pending_distribution_timer(fb::game::server& server);

    async::task<void> handle(const fb::model::datetime& now, std::thread::id id) override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_STORAGE_PENDING_DISTRIBUTION_TIMER_H

