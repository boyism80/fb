#include <fb/game/handler/protocol/miss.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
miss<V>::miss(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::miss<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> miss<V>::handle(fb::socket<character>& session, game_reqs::miss<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    auto obj = map->objects[request.oid];
    if (obj == nullptr)
        co_return true;

    obj->update_external(*ch, false);
    fb::logger::info("Object miss for {}", obj->oid());

    co_return true;
}

template class miss<fb::protocol::CLIENT_VERSION::v550>;
template class miss<fb::protocol::CLIENT_VERSION::v565>;
template class miss<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
