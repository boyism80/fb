#include <fb/game/handler/protocol/user_info_submit.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> user_info_submit<V>::handle(fb::socket<character>& session, game_reqs::user_info_submit<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    // Legacy NHN profile submit — no persistence in FB private server.
    std::ignore = request;
    co_return true;
}

template class user_info_submit<fb::protocol::CLIENT_VERSION::v550>;
template class user_info_submit<fb::protocol::CLIENT_VERSION::v565>;
template class user_info_submit<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
