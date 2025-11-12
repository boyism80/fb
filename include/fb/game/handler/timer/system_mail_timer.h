#ifndef FB_GAME_HANDLER_TIMER_SYSTEM_MAIL_TIMER_H
#define FB_GAME_HANDLER_TIMER_SYSTEM_MAIL_TIMER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::timer {

/**
 * @brief Global timer handler for fetching system mails from internal server.
 * Periodically polls the internal server to retrieve active system mails and stores them in memory.
 */
class system_mail_timer : public fb::handler::timer<fb::game::server>
{
public:
    /**
     * @brief Constructs a new system_mail_timer instance.
     * @param server Reference to the game server instance.
     */
    system_mail_timer(fb::game::server& server);

    /**
     * @brief Handles the timer execution.
     * Fetches system mails from the internal server and updates the in-memory cache.
     * @return An async task representing the timer execution.
     */
    async::task<void> handle() override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_SYSTEM_MAIL_TIMER_H

