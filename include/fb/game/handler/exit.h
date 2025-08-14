#ifndef FB_GAME_HANDLER_EXIT_H
#define FB_GAME_HANDLER_EXIT_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class exit : public fb::handler<fb::game::server, fb::protocol::game::request::exit>
{
public:
    exit(fb::game::server& server);
    exit(const exit&)             = delete;
    exit(exit&&)                  = delete;
    exit& operator= (const exit&) = delete;
    exit& operator= (exit&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::exit& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_EXIT_H
