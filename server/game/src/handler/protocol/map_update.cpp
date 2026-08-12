#include <fb/game/handler/protocol/map_update.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
map_update<V>::map_update(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::map_update<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> map_update<V>::handle(fb::socket<character>& session, game_reqs::map_update<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr)
        co_return true;

    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    ch->update_map(*map, request.begin, request.size, request.crc);
    co_return true;
}

template class map_update<fb::protocol::CLIENT_VERSION::v550>;
template class map_update<fb::protocol::CLIENT_VERSION::v565>;
template class map_update<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
