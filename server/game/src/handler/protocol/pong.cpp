#include <fb/game/handler/protocol/pong.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> pong<V>::handle(fb::socket<character>& session, game_reqs::pong<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr)
        co_return true;

    auto& state = ch->ping_state();
    if (request.token != state.token)
        co_return false;

    state.pong_received = true;
    co_return true;
}

template class pong<fb::protocol::CLIENT_VERSION::v550>;
template class pong<fb::protocol::CLIENT_VERSION::v565>;
template class pong<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
