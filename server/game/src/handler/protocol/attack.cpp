#include <fb/game/handler/protocol/attack.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
attack<V>::attack(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::attack<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> attack<V>::handle(fb::socket<character>& session, game_reqs::attack<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    co_await ch->attack();
    co_return true;
}

template class attack<fb::protocol::CLIENT_VERSION::v550>;
template class attack<fb::protocol::CLIENT_VERSION::v565>;
template class attack<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
