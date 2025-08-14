#ifndef FB_GAME_HANDLER_DIALOG_H
#define FB_GAME_HANDLER_DIALOG_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class dialog : public fb::handler<fb::game::server, fb::protocol::game::request::dialog>
{
public:
    dialog(fb::game::server& server);
    dialog(const dialog&)             = delete;
    dialog(dialog&&)                  = delete;
    dialog& operator= (const dialog&) = delete;
    dialog& operator= (dialog&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::dialog& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_DIALOG_H
