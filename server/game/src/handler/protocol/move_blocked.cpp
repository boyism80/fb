#include <fb/game/handler/protocol/move_blocked.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
move_blocked<V>::move_blocked(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::move_blocked<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> move_blocked<V>::handle(fb::socket<character>& session, game_reqs::move_blocked<V>& request)
{
    co_return true;
}

template class move_blocked<fb::protocol::CLIENT_VERSION::v550>;
template class move_blocked<fb::protocol::CLIENT_VERSION::v565>;
template class move_blocked<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
