#ifndef FB_GAME_HANDLER_FRIENDS_H
#define FB_GAME_HANDLER_FRIENDS_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class friends : public fb::handler::protocol<fb::game::server, game_reqs::friends<V>>
{
public:
    friends(fb::game::server& server);
    friends(const friends&)             = delete;
    friends(friends&&)                  = delete;
    friends& operator= (const friends&) = delete;
    friends& operator= (friends&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::friends<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_FRIENDS_H
