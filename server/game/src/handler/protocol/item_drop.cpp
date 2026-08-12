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
    if (ch->inited() == false)
        co_return true;

    std::ignore = co_await ch->items.drop(request.index, request.all ? -1 : 1);
    co_return true;
}

template class item_drop<fb::protocol::CLIENT_VERSION::v550>;
template class item_drop<fb::protocol::CLIENT_VERSION::v565>;
template class item_drop<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
