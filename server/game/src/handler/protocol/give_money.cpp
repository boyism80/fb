#include <fb/game/handler/protocol/give_money.h>
#include <fb/game/server.h>
#include <tuple>
#include <limits>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
give_money<V>::give_money(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::give_money<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> give_money<V>::handle(fb::socket<character>& session, game_reqs::give_money<V>& request)
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
        auto money = std::min<uint64_t>(request.money, me->money());
        switch (forward->what())
        {
        case OBJECT_TYPE::CHARACTER:
        {
            auto you      = std::static_pointer_cast<fb::game::character>(forward);
            auto capacity = std::numeric_limits<uint64_t>::max() - you->money();
            money         = std::min(capacity, static_cast<uint64_t>(money));
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

template class give_money<fb::protocol::CLIENT_VERSION::v550>;
template class give_money<fb::protocol::CLIENT_VERSION::v565>;
template class give_money<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
