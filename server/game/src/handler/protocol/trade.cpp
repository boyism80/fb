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
    if (me == nullptr || me->inited() == false)
        co_return true;

    auto map = me->map();
    if (map == nullptr)
        co_return true;

    if (request.action == fb::game::trade::state::REQUEST)
    {
        auto you = map->objects[request.oid];
        if (you != nullptr && you->is(OBJECT_TYPE::CHARACTER) && you->hidden(*me) == false)
            std::ignore = fb::game::trade::begin(*me, *you->template shared_from_this_as<character>());
    }
    else
    {
        // Ending the trade releases the character's reference, so the session is held here until the action returns.
        auto trade = me->trade;
        if (trade == nullptr)
            co_return true;

        switch (request.action)
        {
        case fb::game::trade::state::UP_ITEM:
            std::ignore = trade->up_item(*me, request.parameter.index - 1);
            break;

        case fb::game::trade::state::ITEM_COUNT:
            std::ignore = trade->count(*me, request.parameter.count);
            break;

        case fb::game::trade::state::UP_MONEY:
            std::ignore = trade->up_money(*me, request.parameter.money);
            break;

        case fb::game::trade::state::CANCEL:
            std::ignore = trade->cancel(*me);
            break;

        case fb::game::trade::state::LOCK:
            std::ignore = co_await trade->lock(*me);
            break;

        default:
            break;
        }
    }

    co_return true;
}

template class trade<fb::protocol::CLIENT_VERSION::v550>;
template class trade<fb::protocol::CLIENT_VERSION::v565>;
template class trade<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
