#ifndef FB_GAME_HANDLER_TRADE_H
#define FB_GAME_HANDLER_TRADE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class trade : public fb::handler::protocol<fb::game::server, game_reqs::trade<V>>
{
public:
    trade(fb::game::server& server);
    trade(const trade&)             = delete;
    trade(trade&&)                  = delete;
    trade& operator= (const trade&) = delete;
    trade& operator= (trade&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::trade<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_TRADE_H
