#include <fb/game/server.h>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

void listener_impl::on_action(life& me, ACTION action, DURATION duration, uint8_t sound)
{
    this->server.send(me, game_resp::action(me, action, duration), scope::PIVOT);
}

void listener_impl::on_attack(life& me, DURATION duration)
{
    // Listener only handles packet response - no game logic
}

void listener_impl::on_dead(life& me, std::shared_ptr<object> you)
{
    // Listener only handles packet response - no game logic
}

void listener_impl::on_update_hp(life& me, uint64_t diff, bool critical)
{
    this->server.send(me, game_resp::update_hp(me, diff, critical), scope::PIVOT);
}