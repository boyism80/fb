#ifndef FB_GAME_HANDLER_LOOT_H
#define FB_GAME_HANDLER_LOOT_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class loot : public fb::handler<fb::game::server, fb::protocol::game::request::loot>
{
public:
    loot(fb::game::server& server);
    loot(const loot&)             = delete;
    loot(loot&&)                  = delete;
    loot& operator= (const loot&) = delete;
    loot& operator= (loot&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::loot& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_LOOT_H
