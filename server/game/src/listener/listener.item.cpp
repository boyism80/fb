#include <fb/game/server.h>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

void listener_impl::on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr)
{
    me.send(game_resp::item_remove(attr, index, 0));
}

void listener_impl::on_item_update(character& me, uint8_t index)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::item_update<V>(me, index));
    });
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
        this->server.send(me, game_resp::item_throws(me, item, to), scope::PIVOT);
    else
        this->server.send(me, game_resp::action(me, ACTION::THROW, DURATION::THROW), scope::PIVOT);
}