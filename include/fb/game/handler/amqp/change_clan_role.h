#ifndef FB_GAME_HANDLER_AMQP_CHANGE_CLAN_ROLE_H
#define FB_GAME_HANDLER_AMQP_CHANGE_CLAN_ROLE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class change_clan_role : public fb::amqp_handler<fb::game::server, internal_resp::ChangeClanRole>
{
public:
    change_clan_role(fb::game::server& server);
    change_clan_role(const change_clan_role&)             = delete;
    change_clan_role(change_clan_role&&)                  = delete;
    change_clan_role& operator= (const change_clan_role&) = delete;
    change_clan_role& operator= (change_clan_role&&)      = delete;

public:
    async::task<void> handle(const internal_resp::ChangeClanRole& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_CHANGE_CLAN_ROLE_H
