#include <fb/game/handler/protocol/direction.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
direction<V>::direction(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::direction<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> direction<V>::handle(fb::socket<character>& session, game_reqs::direction<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->direction(request.value) == false)
        co_return false;

    co_return true;
}

template class direction<fb::protocol::CLIENT_VERSION::v550>;
template class direction<fb::protocol::CLIENT_VERSION::v565>;
template class direction<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
