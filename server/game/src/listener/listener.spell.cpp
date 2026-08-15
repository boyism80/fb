#include <fb/game/server.h>
#include <fb/game/spell.h>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

void listener_impl::on_spell_update(life& me, uint8_t index)
{
    if (me.is(OBJECT_TYPE::CHARACTER) == false)
        return;

    auto& ch = static_cast<character&>(me);
    fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        ch.send(game_resp::spell_update<V>(me, index));
    });
}

void listener_impl::on_spell_remove(life& me, uint8_t index)
{
    me.send(game_resp::spell_remove(me, index));
}

void listener_impl::on_spell_delay(life& me, const spell& spell, uint32_t delay)
{
    me.send(fb::protocol::game::response::spell_delay(spell, delay));
}