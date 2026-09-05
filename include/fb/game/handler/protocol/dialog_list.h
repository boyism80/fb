#ifndef FB_GAME_HANDLER_DIALOG_LIST_H
#define FB_GAME_HANDLER_DIALOG_LIST_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class dialog_list : public fb::handler::protocol<fb::game::server, game_reqs::dialog_list<V>>
{
public:
    dialog_list(fb::game::server& server);
    dialog_list(const dialog_list&)             = delete;
    dialog_list(dialog_list&&)                  = delete;
    dialog_list& operator= (const dialog_list&) = delete;
    dialog_list& operator= (dialog_list&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::dialog_list<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_DIALOG_LIST_H
