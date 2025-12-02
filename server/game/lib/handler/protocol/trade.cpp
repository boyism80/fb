#include <fb/game/handler/protocol/trade.h>
#include <fb/game/server.h>

fb::game::handler::protocol::trade::trade(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::trade>(server)
{ }

async::task<bool> fb::game::handler::protocol::trade::handle(fb::socket<character>& session, fb::protocol::game::request::trade& request)
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
    {
        me->trade.begin(you->shared_from_this_as<character>());
        break;
    }

    case fb::game::trade::state::UP_ITEM:
    {
        auto index = request.parameter.index - 1;
        me->trade.up_item(index);
        break;
    }

    case fb::game::trade::state::ITEM_COUNT:
    {
        me->trade.count(request.parameter.count);
        break;
    }

    case fb::game::trade::state::UP_MONEY:
    {
        me->trade.up_money(request.parameter.money);
        break;
    }

    case fb::game::trade::state::CANCEL:
    {
        me->trade.cancel();
        break;
    }

    case fb::game::trade::state::LOCK:
    {
        me->trade.lock();
        break;
    }
    }

    co_return true;
}
