#ifndef FB_GAME_HANDLER_CLICK_H
#define FB_GAME_HANDLER_CLICK_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class click : public fb::handler::protocol<fb::game::server, game_reqs::click>
{
public:
    click(fb::game::server& server);
    click(const click&)             = delete;
    click(click&&)                  = delete;
    click& operator= (const click&) = delete;
    click& operator= (click&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::click& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_CLICK_H
