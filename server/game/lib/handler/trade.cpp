#include <fb/game/handler/trade.h>
#include <fb/game/server.h>

fb::game::handler::trade::trade(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::trade>(server)
{ }

async::task<bool> fb::game::handler::trade::handle(fb::socket<character>&              session,
                                                   fb::protocol::game::request::trade& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    auto map = me->map();
    if (map == nullptr)
        co_return true;

    auto you = static_cast<character*>(map->objects[request.oid]); // 파트너
    if (you == nullptr)
        co_return true;

    switch (request.action)
    {
    case fb::game::trade::state::REQUEST:
    {
        me->trade.begin(you->shared_from_this_as<character>());
        break;
    }

    case fb::game::trade::state::UP_ITEM: // 아이템 올릴때
    {
        auto index = request.parameter.index - 1;
        me->trade.up_item(index);
        break;
    }

    case fb::game::trade::state::ITEM_COUNT: // 아이템 갯수까지 해서 올릴 때
    {
        me->trade.count(request.parameter.count);
        break;
    }

    case fb::game::trade::state::UP_MONEY: // 금전 올릴 때
    {
        // 클라이언트가 입력한 금전 양
        me->trade.up_money(request.parameter.money);
        break;
    }

    case fb::game::trade::state::CANCEL: // 취소한 경우
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
