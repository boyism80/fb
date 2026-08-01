#ifndef FB_GAME_HANDLER_DIALOG_LIST_H
#define FB_GAME_HANDLER_DIALOG_LIST_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class dialog_list : public fb::handler::protocol<fb::game::server, game_reqs::dialog_list>
{
public:
    dialog_list(fb::game::server& server);
    dialog_list(const dialog_list&)             = delete;
    dialog_list(dialog_list&&)                  = delete;
    dialog_list& operator= (const dialog_list&) = delete;
    dialog_list& operator= (dialog_list&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::dialog_list& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_DIALOG_LIST_H
