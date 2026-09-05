#ifndef FB_GAME_HANDLER_FRONT_INFO_H
#define FB_GAME_HANDLER_FRONT_INFO_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class front_info : public fb::handler::protocol<fb::game::server, game_reqs::front_info<V>>
{
public:
    front_info(fb::game::server& server);
    front_info(const front_info&)             = delete;
    front_info(front_info&&)                  = delete;
    front_info& operator= (const front_info&) = delete;
    front_info& operator= (front_info&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::front_info<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_FRONT_INFO_H
