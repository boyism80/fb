#ifndef FB_GAME_HANDLER_USER_LIST_H
#define FB_GAME_HANDLER_USER_LIST_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class user_list : public fb::handler::protocol<fb::game::server, game_reqs::user_list<V>>
{
public:
    user_list(fb::game::server& server);
    user_list(const user_list&)             = delete;
    user_list(user_list&&)                  = delete;
    user_list& operator= (const user_list&) = delete;
    user_list& operator= (user_list&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::user_list<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_USER_LIST_H
