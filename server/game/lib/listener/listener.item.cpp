#include <fb/game/server.h>

using namespace fb::game;

void listener_impl::on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr)
{
    me.send(fb_resp::item_remove(attr, index, 0));
}

void listener_impl::on_item_update(character& me, uint8_t index)
{
    me.send(fb_resp::item_update(me, index));
}

void listener_impl::on_item_swap(character& me, uint8_t src, uint8_t dst)
{
    return;
}

void listener_impl::on_item_active(character& me, item& item)
{
    // Listener only handles packet response - no game logic
}

void listener_impl::on_item_throws(character& me, item& item, const fb::model::point16_t& to)
{
    if (me.position() != to)
        this->server.send(me, fb_resp::item_throws(me, item, to), scope::PIVOT);
    else
        this->server.send(me, fb_resp::action(me, ACTION::ATTACK, DURATION::THROW), scope::PIVOT);
}