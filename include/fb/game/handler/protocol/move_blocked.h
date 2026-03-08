#ifndef FB_GAME_HANDLER_PROTOCOL_MOVE_BLOCKED_H
#define FB_GAME_HANDLER_PROTOCOL_MOVE_BLOCKED_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

/**
 * Handler for client packet 0x69 (move blocked).
 * Performs no action; packet is consumed and ignored (same as original map server).
 */
class move_blocked : public fb::handler::protocol<fb::game::server, game_reqs::move_blocked>
{
public:
    move_blocked(fb::game::server& server);
    move_blocked(const move_blocked&)             = delete;
    move_blocked(move_blocked&&)                  = delete;
    move_blocked& operator= (const move_blocked&) = delete;
    move_blocked& operator= (move_blocked&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::move_blocked& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_PROTOCOL_MOVE_BLOCKED_H
