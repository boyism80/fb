#include <fb/game/handler/protocol/item_drop.h>

#include <fb/game/server.h>

#include <tuple>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
item_drop<V>::item_drop(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_drop<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> item_drop<V>::handle(fb::socket<character>& session, game_reqs::item_drop<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    if (ch->trade != nullptr)
    {
        ch->message(_TEXT(MESSAGE_TRADE_BLOCKED_WHILE_TRADING));
        co_return true;
    }

    auto item = ch->items.at(request.index);
    if (item == nullptr && ch->items.reserved(request.index))
    {
        ch->message(_TEXT(MESSAGE_MARKETPLACE_ITEM_LOCKED));
        co_return true;
    }

    auto count = uint16_t{1};
    if (request.all && item != nullptr)
        count = item->count();
    std::ignore = co_await ch->items.drop(request.index, count);
    co_return true;
}

template class item_drop<fb::protocol::CLIENT_VERSION::v550>;
template class item_drop<fb::protocol::CLIENT_VERSION::v565>;
template class item_drop<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
