#ifndef FB_GAME_HANDLER_MISS_H
#define FB_GAME_HANDLER_MISS_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class miss : public fb::handler<fb::game::server, fb::protocol::game::request::miss>
{
public:
    miss(fb::game::server& server);
    miss(const miss&)             = delete;
    miss(miss&&)                  = delete;
    miss& operator= (const miss&) = delete;
    miss& operator= (miss&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::miss& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_MISS_H
