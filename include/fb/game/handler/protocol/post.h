#ifndef FB_GAME_HANDLER_POST_H
#define FB_GAME_HANDLER_POST_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class post : public fb::handler::protocol<fb::game::server, game_reqs::post>
{
public:
    post(fb::game::server& server);
    post(const post&)             = delete;
    post(post&&)                  = delete;
    post& operator= (const post&) = delete;
    post& operator= (post&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::post& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_POST_H
