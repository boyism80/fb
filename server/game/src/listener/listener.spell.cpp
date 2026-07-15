#include <fb/game/server.h>
#include <fb/game/spell.h>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

void listener_impl::on_spell_update(life& me, uint8_t index)
{
    me.send(game_resp::spell_update(me, index));
}

void listener_impl::on_spell_remove(life& me, uint8_t index)
{
    me.send(game_resp::spell_remove(me, index));
}

void listener_impl::on_spell_delay(life& me, const spell& spell, uint32_t delay)
{
    me.send(fb::protocol::game::response::spell_delay(spell, delay));
}