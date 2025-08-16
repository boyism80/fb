#ifndef FB_GAME_HANDLER_AMQP_SET_CLAN_TITLE_H
#define FB_GAME_HANDLER_AMQP_SET_CLAN_TITLE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class set_clan_title : public fb::handler::amqp<fb::game::server, internal_resp::SetClanTitle>
{
public:
    set_clan_title(fb::game::server& server);
    set_clan_title(const set_clan_title&)             = delete;
    set_clan_title(set_clan_title&&)                  = delete;
    set_clan_title& operator= (const set_clan_title&) = delete;
    set_clan_title& operator= (set_clan_title&&)      = delete;

public:
    async::task<void> handle(const internal_resp::SetClanTitle& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_SET_CLAN_TITLE_H
