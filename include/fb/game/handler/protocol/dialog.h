#ifndef FB_GAME_HANDLER_DIALOG_H
#define FB_GAME_HANDLER_DIALOG_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class dialog : public fb::handler::protocol<fb::game::server, game_reqs::dialog<V>>
{
public:
    dialog(fb::game::server& server);
    dialog(const dialog&)             = delete;
    dialog(dialog&&)                  = delete;
    dialog& operator= (const dialog&) = delete;
    dialog& operator= (dialog&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::dialog<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_DIALOG_H
