#ifndef FB_GAME_HANDLER_MISS_H
#define FB_GAME_HANDLER_MISS_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class miss : public fb::handler::protocol<fb::game::server, game_reqs::miss>
{
public:
    miss(fb::game::server& server);
    miss(const miss&)             = delete;
    miss(miss&&)                  = delete;
    miss& operator= (const miss&) = delete;
    miss& operator= (miss&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::miss& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_MISS_H
