#ifndef FB_GAME_HANDLER_GROUP_H
#define FB_GAME_HANDLER_GROUP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class group : public fb::handler<fb::game::server, fb::protocol::game::request::group>
{
public:
    group(fb::game::server& server);
    group(const group&)             = delete;
    group(group&&)                  = delete;
    group& operator= (const group&) = delete;
    group& operator= (group&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::group& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_GROUP_H
