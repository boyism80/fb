#ifndef FB_GAME_HANDLER_SPELL_CAST_H
#define FB_GAME_HANDLER_SPELL_CAST_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class spell_cast : public fb::handler::protocol<fb::game::server, game_reqs::spell_cast<V>>
{
public:
    spell_cast(fb::game::server& server);
    spell_cast(const spell_cast&)             = delete;
    spell_cast(spell_cast&&)                  = delete;
    spell_cast& operator= (const spell_cast&) = delete;
    spell_cast& operator= (spell_cast&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::spell_cast<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_SPELL_CAST_H
