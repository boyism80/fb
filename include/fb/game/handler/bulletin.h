#ifndef FB_GAME_HANDLER_BULLETIN_H
#define FB_GAME_HANDLER_BULLETIN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class bulletin : public fb::handler<fb::game::server, fb::protocol::game::request::bulletin>
{
public:
    bulletin(fb::game::server& server);
    bulletin(const bulletin&)             = delete;
    bulletin(bulletin&&)                  = delete;
    bulletin& operator= (const bulletin&) = delete;
    bulletin& operator= (bulletin&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::bulletin& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_BULLETIN_H
