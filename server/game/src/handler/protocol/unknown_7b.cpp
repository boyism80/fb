#include <fb/game/handler/protocol/unknown_7b.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> unknown_7b<V>::handle(fb::socket<character>& session, game_reqs::unknown_7b<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    fb::logger::info("unknown_7b name={} from {}", request.name, ch->name());
    co_return true;
}

template class unknown_7b<fb::protocol::CLIENT_VERSION::v550>;
template class unknown_7b<fb::protocol::CLIENT_VERSION::v565>;
template class unknown_7b<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
