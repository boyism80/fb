#ifndef FB_GAME_HANDLER_SPELL_CAST_H
#define FB_GAME_HANDLER_SPELL_CAST_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class spell_cast : public fb::handler<fb::game::server, fb::protocol::game::request::spell_cast>
{
public:
    spell_cast(fb::game::server& server);
    spell_cast(const spell_cast&)             = delete;
    spell_cast(spell_cast&&)                  = delete;
    spell_cast& operator= (const spell_cast&) = delete;
    spell_cast& operator= (spell_cast&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::spell_cast& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_SPELL_CAST_H
