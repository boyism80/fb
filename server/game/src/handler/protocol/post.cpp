#include <fb/game/handler/protocol/post.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
post<V>::post(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::post<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> post<V>::handle(fb::socket<character>& session, game_reqs::post<V>& request)
{
    co_return true;
}

template class post<fb::protocol::CLIENT_VERSION::v550>;
template class post<fb::protocol::CLIENT_VERSION::v565>;
template class post<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
