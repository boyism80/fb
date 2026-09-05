#ifndef FB_GAME_HANDLER_PROTOCOL_USER_INFO_SUBMIT_H
#define FB_GAME_HANDLER_PROTOCOL_USER_INFO_SUBMIT_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

/**
 * Handler for C2S user_info_submit (0x53) from USERINFO dialog OK.
 * Legacy profile registration; accepted and ignored for private-server play.
 */
template <fb::protocol::CLIENT_VERSION V>
class user_info_submit : public fb::handler::protocol<fb::game::server, game_reqs::user_info_submit<V>>
{
public:
    user_info_submit(fb::game::server& server) :
        fb::handler::protocol<fb::game::server, game_reqs::user_info_submit<V>>(server)
    { }
    user_info_submit(const user_info_submit&)             = delete;
    user_info_submit(user_info_submit&&)                  = delete;
    user_info_submit& operator= (const user_info_submit&) = delete;
    user_info_submit& operator= (user_info_submit&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::user_info_submit<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif
