#ifndef FB_GAME_HANDLER_FRIENDS_H
#define FB_GAME_HANDLER_FRIENDS_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class friends : public fb::handler::protocol<fb::game::server, fb::protocol::game::request::friends>
{
public:
    friends(fb::game::server& server);
    friends(const friends&)             = delete;
    friends(friends&&)                  = delete;
    friends& operator= (const friends&) = delete;
    friends& operator= (friends&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::friends& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_FRIENDS_H
