#ifndef FB_GAME_HANDLER_GIVE_MONEY_H
#define FB_GAME_HANDLER_GIVE_MONEY_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class give_money : public fb::handler<fb::game::server, fb::protocol::game::request::give_money>
{
public:
    give_money(fb::game::server& server);
    give_money(const give_money&)             = delete;
    give_money(give_money&&)                  = delete;
    give_money& operator= (const give_money&) = delete;
    give_money& operator= (give_money&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::give_money& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_GIVE_MONEY_H
