#include <fb/game/handler/protocol/loot.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
loot<V>::loot(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::loot<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> loot<V>::handle(fb::socket<character>& session, game_reqs::loot<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    co_await ch->items.loot(request.boost);
    co_return true;
}

template class loot<fb::protocol::CLIENT_VERSION::v550>;
template class loot<fb::protocol::CLIENT_VERSION::v565>;
template class loot<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
