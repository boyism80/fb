#ifndef FB_GAME_HANDLER_USER_LIST_H
#define FB_GAME_HANDLER_USER_LIST_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class user_list : public fb::handler::protocol<fb::game::server, fb::protocol::game::request::user_list>
{
public:
    user_list(fb::game::server& server);
    user_list(const user_list&)             = delete;
    user_list(user_list&&)                  = delete;
    user_list& operator= (const user_list&) = delete;
    user_list& operator= (user_list&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::user_list& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_USER_LIST_H
