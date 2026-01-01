#ifndef FB_GAME_HANDLER_GROUP_H
#define FB_GAME_HANDLER_GROUP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class group : public fb::handler::protocol<fb::game::server, game_reqs::group>
{
public:
    group(fb::game::server& server);
    group(const group&)             = delete;
    group(group&&)                  = delete;
    group& operator= (const group&) = delete;
    group& operator= (group&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::group& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_GROUP_H
