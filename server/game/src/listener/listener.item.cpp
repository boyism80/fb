#include <fb/game/server.h>
#include <tuple>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

async::task<void> listener_impl::on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr)
{
    std::ignore = co_await me.send(game_resp::item_remove(attr, index, 0));
}

async::task<void> listener_impl::on_item_update(character& me, uint8_t index)
{
    std::ignore = co_await me.send(game_resp::item_update(me, index));
}

async::task<void> listener_impl::on_item_swap(character& me, uint8_t src, uint8_t dst)
{
    co_return;
}

async::task<void> listener_impl::on_item_active(character& me, item& item)
{
    // Listener only handles packet response - no game logic
    co_return;
}

async::task<void> listener_impl::on_item_throws(character& me, item& item, const fb::model::point16_t& to)
{
    if (me.position() != to)
        co_await this->server.send(me, game_resp::item_throws(me, item, to), scope::PIVOT);
    else
        co_await this->server.send(me, game_resp::action(me, ACTION::ATTACK, DURATION::THROW), scope::PIVOT);
}
