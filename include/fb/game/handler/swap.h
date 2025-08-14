#ifndef FB_GAME_HANDLER_SWAP_H
#define FB_GAME_HANDLER_SWAP_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class swap : public fb::handler<fb::game::server, fb::protocol::game::request::swap>
{
public:
    swap(fb::game::server& server);
    swap(const swap&)             = delete;
    swap(swap&&)                  = delete;
    swap& operator= (const swap&) = delete;
    swap& operator= (swap&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::swap& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_SWAP_H
