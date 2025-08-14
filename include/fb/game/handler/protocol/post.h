#ifndef FB_GAME_HANDLER_POST_H
#define FB_GAME_HANDLER_POST_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class post : public fb::handler<fb::game::server, fb::protocol::game::request::post>
{
public:
    post(fb::game::server& server);
    post(const post&)             = delete;
    post(post&&)                  = delete;
    post& operator= (const post&) = delete;
    post& operator= (post&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::post& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_POST_H
