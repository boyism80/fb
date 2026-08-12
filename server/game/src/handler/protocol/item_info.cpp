#include <fb/game/handler/protocol/item_info.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
item_info<V>::item_info(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_info<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> item_info<V>::handle(fb::socket<character>& session, game_reqs::item_info<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto item = ch->items[request.slot];
    if (item == nullptr)
        co_return false;

    ch->item_tooltip(*item, request.position);
    co_return true;
}

template class item_info<fb::protocol::CLIENT_VERSION::v550>;
template class item_info<fb::protocol::CLIENT_VERSION::v565>;
template class item_info<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
