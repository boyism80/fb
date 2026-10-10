#ifndef FB_GAME_HANDLER_PROTOCOL_REPUTATION_H
#define FB_GAME_HANDLER_PROTOCOL_REPUTATION_H

#include <fb/game/protocol/reputation.h>
#include <fb/game/server.h>
#include <fb/handler.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class reputation : public fb::handler::protocol<fb::game::server, game_reqs::reputation<V>>
{
public:
    reputation(fb::game::server& server) :
        fb::handler::protocol<fb::game::server, game_reqs::reputation<V>>(server)
    { }
    reputation(const reputation&)             = delete;
    reputation(reputation&&)                  = delete;
    reputation& operator= (const reputation&) = delete;
    reputation& operator= (reputation&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::reputation<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif
