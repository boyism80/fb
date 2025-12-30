#ifndef FB_GAME_HANDLER_ATTACK_H
#define FB_GAME_HANDLER_ATTACK_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class attack : public fb::handler::protocol<fb::game::server, game_reqs::attack>
{
public:
    attack(fb::game::server& server);
    attack(const attack&)             = delete;
    attack(attack&&)                  = delete;
    attack& operator= (const attack&) = delete;
    attack& operator= (attack&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::attack& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_ATTACK_H
