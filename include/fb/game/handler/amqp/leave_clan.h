#ifndef FB_GAME_HANDLER_AMQP_LEAVE_CLAN_H
#define FB_GAME_HANDLER_AMQP_LEAVE_CLAN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class leave_clan : public fb::amqp_handler<fb::game::server, internal_resp::LeaveClan>
{
public:
    leave_clan(fb::game::server& server);
    leave_clan(const leave_clan&)             = delete;
    leave_clan(leave_clan&&)                  = delete;
    leave_clan& operator= (const leave_clan&) = delete;
    leave_clan& operator= (leave_clan&&)      = delete;

public:
    async::task<void> handle(const internal_resp::LeaveClan& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_LEAVE_CLAN_H
