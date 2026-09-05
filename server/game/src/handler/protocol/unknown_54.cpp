#include <fb/game/handler/protocol/unknown_54.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> unknown_54<V>::handle(fb::socket<character>& session, game_reqs::unknown_54<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    fb::logger::info("unknown_54 phase={} token={:#x} from {}", request.phase, request.token, ch->name());
    co_return true;
}

template class unknown_54<fb::protocol::CLIENT_VERSION::v550>;
template class unknown_54<fb::protocol::CLIENT_VERSION::v565>;
template class unknown_54<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
