#include <fb/game/handler/give_money.h>
#include <fb/game/server.h>

using namespace fb::game::handler;

give_money::give_money(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::give_money>(server)
{ }

async::task<bool> give_money::handle(fb::socket<character>& session, fb::protocol::game::request::give_money& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    if (me->map() == nullptr)
        co_return true;

    auto forward = me->forward();
    if (forward == nullptr)
        co_return true;

    try
    {
        auto money = std::min(request.money, me->money());
        switch (forward->what())
        {
        case OBJECT_TYPE::CHARACTER:
        {
            auto you      = std::static_pointer_cast<fb::game::character>(forward);
            auto capacity = 0xFFFFFFFF - you->money();
            money         = std::min(capacity, money);
            if (money == 0)
                throw std::runtime_error("상대방이 돈을 받을 수 없습니다.");

            you->money_add(money);
            you->message(std::format("{}님이 {}전을 주었습니다.", me->name(), money));
        }
        break;

        case OBJECT_TYPE::MOB:
        {
            auto mob = std::static_pointer_cast<fb::game::mob>(forward);
            if (mob->items().size() >= CONTAINER_CAPACITY)
                throw std::runtime_error("더 이상 줄 수 없습니다.");

            auto item = this->server.make<fb::game::cash>(money);
            mob->push_item(item);
        }
        break;

        default:
            co_return true;
        }

        me->money_reduce(money);
    }
    catch (std::exception& e)
    {
        me->message(e.what());
    }
    co_return true;
}
