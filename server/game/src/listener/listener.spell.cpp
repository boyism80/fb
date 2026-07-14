#include <fb/game/server.h>
#include <fb/game/spell.h>
#include <tuple>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

async::task<void> listener_impl::on_spell_update(life& me, uint8_t index)
{
    std::ignore = co_await me.send(game_resp::spell_update(me, index));
}

async::task<void> listener_impl::on_spell_remove(life& me, uint8_t index)
{
    std::ignore = co_await me.send(game_resp::spell_remove(me, index));
}

async::task<void> listener_impl::on_spell_delay(life& me, const spell& spell, uint32_t delay)
{
    std::ignore = co_await me.send(fb::protocol::game::response::spell_delay(spell, delay));
}
