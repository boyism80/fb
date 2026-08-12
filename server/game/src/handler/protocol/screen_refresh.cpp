#include <fb/game/handler/protocol/screen_refresh.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
screen_refresh<V>::screen_refresh(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::screen_refresh<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> screen_refresh<V>::handle(fb::socket<character>& session, game_reqs::screen_refresh<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    ch->screen_refresh();
    co_return true;
}

template class screen_refresh<fb::protocol::CLIENT_VERSION::v550>;
template class screen_refresh<fb::protocol::CLIENT_VERSION::v565>;
template class screen_refresh<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
