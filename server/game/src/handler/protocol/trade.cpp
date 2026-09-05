#include <fb/game/handler/protocol/trade.h>
#include <fb/game/server.h>
#include <tuple>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
trade<V>::trade(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::trade<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> trade<V>::handle(fb::socket<character>& session, game_reqs::trade<V>& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    auto map = me->map();
    if (map == nullptr)
        co_return true;

    auto you = static_cast<character*>(map->objects[request.oid]);
    if (you == nullptr)
        co_return true;

    switch (request.action)
    {
    case fb::game::trade::state::REQUEST:
        std::ignore = me->trade.begin(you->shared_from_this_as<character>());
        break;

    case fb::game::trade::state::UP_ITEM:
        std::ignore = me->trade.up_item(request.parameter.index - 1);
        break;

    case fb::game::trade::state::ITEM_COUNT:
        std::ignore = me->trade.count(request.parameter.count);
        break;

    case fb::game::trade::state::UP_MONEY:
        std::ignore = me->trade.up_money(request.parameter.money);
        break;

    case fb::game::trade::state::CANCEL:
        std::ignore = me->trade.cancel();
        break;

    case fb::game::trade::state::LOCK:
        std::ignore = co_await me->trade.lock();
        break;
    }

    co_return true;
}

template class trade<fb::protocol::CLIENT_VERSION::v550>;
template class trade<fb::protocol::CLIENT_VERSION::v565>;
template class trade<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
