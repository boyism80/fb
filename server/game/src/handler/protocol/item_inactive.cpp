#include <fb/game/handler/protocol/item_inactive.h>
#include <fb/game/server.h>
#include <tuple>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
item_inactive<V>::item_inactive(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_inactive<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> item_inactive<V>::handle(fb::socket<character>& session, game_reqs::item_inactive<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->items.free_size() == 0)
    {
        ch->message(_TEXT(MESSAGE_EXCEPTION_INVENTORY_OVERFLOW));
        co_return true;
    }

    std::ignore = co_await ch->items.inactive(request.parts);
    co_return true;
}

template class item_inactive<fb::protocol::CLIENT_VERSION::v550>;
template class item_inactive<fb::protocol::CLIENT_VERSION::v565>;
template class item_inactive<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
