#include <fb/game/handler/protocol/item_combine.h>

#include <fb/game/server.h>

#include <tuple>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
item_combine<V>::item_combine(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_combine<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> item_combine<V>::handle(fb::socket<character>& session, game_reqs::item_combine<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    if (ch->trade.trading())
    {
        ch->message(_TEXT(MESSAGE_TRADE_BLOCKED_WHILE_TRADING));
        co_return true;
    }

    if (request.indices.size() > CONTAINER_CAPACITY - 1)
        co_return false;

    std::ignore = co_await ch->items.combine(request.indices);
    co_return true;
}

template class item_combine<fb::protocol::CLIENT_VERSION::v550>;
template class item_combine<fb::protocol::CLIENT_VERSION::v565>;
template class item_combine<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
