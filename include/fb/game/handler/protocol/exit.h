#ifndef FB_GAME_HANDLER_EXIT_H
#define FB_GAME_HANDLER_EXIT_H

#include <fb/game/server.h>
#include <fb/handler.h>

#include <cstdlib>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class exit : public fb::handler::protocol<fb::game::server, game_reqs::exit<V>>
{
public:
    exit(fb::game::server& server);
    exit(const exit&)             = delete;
    exit(exit&&)                  = delete;
    exit& operator= (const exit&) = delete;
    exit& operator= (exit&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::exit<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_EXIT_H
