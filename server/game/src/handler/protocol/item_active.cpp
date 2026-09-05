#include <fb/game/handler/protocol/item_active.h>
#include <fb/game/server.h>
#include <tuple>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
item_active<V>::item_active(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_active<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> item_active<V>::handle(fb::socket<character>& session, game_reqs::item_active<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    std::ignore = co_await ch->items.active(request.index);
    co_return true;
}

template class item_active<fb::protocol::CLIENT_VERSION::v550>;
template class item_active<fb::protocol::CLIENT_VERSION::v565>;
template class item_active<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
