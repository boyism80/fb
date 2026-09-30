#ifndef FB_GAME_HANDLER_SELF_INFO_H
#define FB_GAME_HANDLER_SELF_INFO_H

#include <fb/game/server.h>
#include <fb/handler.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class self_info : public fb::handler::protocol<fb::game::server, game_reqs::self_info<V>>
{
public:
    self_info(fb::game::server& server);
    self_info(const self_info&)             = delete;
    self_info(self_info&&)                  = delete;
    self_info& operator= (const self_info&) = delete;
    self_info& operator= (self_info&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::self_info<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_SELF_INFO_H
