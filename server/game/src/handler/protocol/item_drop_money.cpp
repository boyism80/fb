#include <fb/game/handler/protocol/item_drop_money.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
item_drop_money<V>::item_drop_money(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_drop_money<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> item_drop_money<V>::handle(fb::socket<character>& session, game_reqs::item_drop_money<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return false;

    auto chunk = std::min<uint64_t>(ch->money(), request.chunk);

    std::ignore = co_await ch->money_drop(chunk);
    co_return true;
}

template class item_drop_money<fb::protocol::CLIENT_VERSION::v550>;
template class item_drop_money<fb::protocol::CLIENT_VERSION::v565>;
template class item_drop_money<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
