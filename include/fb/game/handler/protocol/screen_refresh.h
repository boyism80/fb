#ifndef FB_GAME_HANDLER_SCREEN_REFRESH_H
#define FB_GAME_HANDLER_SCREEN_REFRESH_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class screen_refresh : public fb::handler::protocol<fb::game::server, game_reqs::screen_refresh<V>>
{
public:
    screen_refresh(fb::game::server& server);
    screen_refresh(const screen_refresh&)             = delete;
    screen_refresh(screen_refresh&&)                  = delete;
    screen_refresh& operator= (const screen_refresh&) = delete;
    screen_refresh& operator= (screen_refresh&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::screen_refresh<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_SCREEN_REFRESH_H
