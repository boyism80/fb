#ifndef FB_GAME_HANDLER_UPDATE_SCREEN_H
#define FB_GAME_HANDLER_UPDATE_SCREEN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class update_screen : public fb::handler<fb::game::server, fb::protocol::game::request::update_screen>
{
public:
    update_screen(fb::game::server& server);
    update_screen(const update_screen&)             = delete;
    update_screen(update_screen&&)                  = delete;
    update_screen& operator= (const update_screen&) = delete;
    update_screen& operator= (update_screen&&)      = delete;

    async::task<bool> handle(fb::socket<character>&                      session,
                             fb::protocol::game::request::update_screen& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_UPDATE_SCREEN_H
