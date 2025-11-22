#ifndef FB_GAME_HANDLER_TIMER_SYSTEM_MAIL_DISTRIBUTION_TIMER_H
#define FB_GAME_HANDLER_TIMER_SYSTEM_MAIL_DISTRIBUTION_TIMER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::timer {

/**
 * @brief Thread timer handler for distributing system mails to logged-in users.
 * This timer processes a limited number of users per execution to prevent overload.
 * It sends unreceived system mails to currently logged-in users in the thread.
 */
class system_mail_distribution_timer : public fb::handler::timer<fb::game::server>
{
public:
    /**
     * @brief Constructs a new system_mail_distribution_timer instance.
     * @param server The game server instance.
     */
    system_mail_distribution_timer(fb::game::server& server);

    /**
     * @brief Handles the timer's execution, distributing system mails to logged-in users.
     * @param now The current datetime.
     * @param id The thread ID where this timer is executing.
     * @return An asynchronous task representing the operation.
     */
    async::task<void> handle(const fb::model::datetime& now, std::thread::id id) override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_SYSTEM_MAIL_DISTRIBUTION_TIMER_H

