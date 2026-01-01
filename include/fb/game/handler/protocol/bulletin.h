#ifndef FB_GAME_HANDLER_BULLETIN_H
#define FB_GAME_HANDLER_BULLETIN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class bulletin : public fb::handler::protocol<fb::game::server, game_reqs::bulletin>
{
public:
    bulletin(fb::game::server& server);
    bulletin(const bulletin&)             = delete;
    bulletin(bulletin&&)                  = delete;
    bulletin& operator= (const bulletin&) = delete;
    bulletin& operator= (bulletin&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::bulletin& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_BULLETIN_H
