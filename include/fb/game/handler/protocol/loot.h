#ifndef FB_GAME_HANDLER_LOOT_H
#define FB_GAME_HANDLER_LOOT_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class loot : public fb::handler::protocol<fb::game::server, game_reqs::loot<V>>
{
public:
    loot(fb::game::server& server);
    loot(const loot&)             = delete;
    loot(loot&&)                  = delete;
    loot& operator= (const loot&) = delete;
    loot& operator= (loot&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::loot<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_LOOT_H
