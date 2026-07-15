#include <fb/game/handler/protocol/give_money.h>
#include <fb/game/server.h>
#include <tuple>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

give_money::give_money(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::give_money>(server)
{ }

async::task<bool> give_money::handle(fb::socket<character>& session, game_reqs::give_money& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    if (me->map() == nullptr)
        co_return true;

    auto forward = me->forward();
    if (forward == nullptr)
        co_return true;

    auto error = std::optional<std::string>{};
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
                throw std::runtime_error(_TEXT(MESSAGE_MONEY_TARGET_CANNOT_RECEIVE));

            std::ignore = you->money_add(money);
            you->message(std::format(_TEXT(MESSAGE_MONEY_GIVE), me->name(), money));
        }
        break;

        case OBJECT_TYPE::MOB:
        {
            auto mob = std::static_pointer_cast<fb::game::mob>(forward);
            if (mob->items().size() >= CONTAINER_CAPACITY)
                throw std::runtime_error(_TEXT(MESSAGE_MONEY_CANNOT_GIVE_ANYMORE));

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
        error = e.what();
    }

    if (error.has_value())
        me->message(error.value());
    co_return true;
}
