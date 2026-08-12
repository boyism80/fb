#include <fb/game/handler/protocol/map_world.h>
#include <fb/game/server.h>

using table = fb::model::table;

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
map_world<V>::map_world(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::map_world<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> map_world<V>::handle(fb::socket<character>& session, game_reqs::map_world<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto  world_table = table::world;
    auto& world       = world_table[request.value];
    auto& before      = world[request.before];
    auto& after       = world[request.after];

    if (ch->map() == this->server.maps[after.map])
    {
        ch->update_map();
        ch->update_external(true);
    }
    else
    {
        std::ignore = co_await ch->map(this->server.maps[after.map], after.position);
    }
    co_return true;
}

template class map_world<fb::protocol::CLIENT_VERSION::v550>;
template class map_world<fb::protocol::CLIENT_VERSION::v565>;
template class map_world<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
