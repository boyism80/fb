#include <fb/game/handler/protocol/self_info.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
self_info<V>::self_info(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::self_info<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> self_info<V>::handle(fb::socket<character>& session, game_reqs::self_info<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
    {
        if (request.party)
        {
            this->server.groups.update_portraits(*ch);
        }
        else
        {
            ch->update_internal();
            ch->update_buff();
        }
    }
    else
    {
        ch->update_internal();
        ch->update_buff();
    }
    co_return true;
}

template class self_info<fb::protocol::CLIENT_VERSION::v550>;
template class self_info<fb::protocol::CLIENT_VERSION::v565>;
template class self_info<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
