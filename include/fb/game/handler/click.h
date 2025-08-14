#ifndef FB_GAME_HANDLER_CLICK_H
#define FB_GAME_HANDLER_CLICK_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class click : public fb::handler<fb::game::server, fb::protocol::game::request::click>
{
public:
    click(fb::game::server& server);
    click(const click&)             = delete;
    click(click&&)                  = delete;
    click& operator= (const click&) = delete;
    click& operator= (click&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::click& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_CLICK_H
