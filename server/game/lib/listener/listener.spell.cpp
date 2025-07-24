#include <fb/game/server.h>

using namespace fb::game;

void listener_impl::on_spell_update(life& me, uint8_t index)
{
    me.send(fb_resp::spell_update(me, index));
}

void listener_impl::on_spell_remove(life& me, uint8_t index)
{
    me.send(fb_resp::spell_remove(me, index));
}