#include <fb/game/handler/protocol/emotion.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
emotion<V>::emotion(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::emotion<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> emotion<V>::handle(fb::socket<character>& session, game_reqs::emotion<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (request.value <= 13 || request.value == 0xFE || request.value == 0xFF)
        ch->action(ACTION(static_cast<int>(ACTION::EMOTION) + request.value), DURATION::EMOTION);
    co_return true;
}

template class emotion<fb::protocol::CLIENT_VERSION::v550>;
template class emotion<fb::protocol::CLIENT_VERSION::v565>;
template class emotion<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
