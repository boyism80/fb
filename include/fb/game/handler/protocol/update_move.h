#ifndef FB_GAME_HANDLER_UPDATE_MOVE_H
#define FB_GAME_HANDLER_UPDATE_MOVE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class update_move : public fb::handler::protocol<fb::game::server, game_reqs::update_move>
{
public:
    update_move(fb::game::server& server);
    update_move(const update_move&)             = delete;
    update_move(update_move&&)                  = delete;
    update_move& operator= (const update_move&) = delete;
    update_move& operator= (update_move&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::update_move& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_UPDATE_MOVE_H
