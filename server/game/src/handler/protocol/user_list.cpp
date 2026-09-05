#include <fb/game/handler/protocol/user_list.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
user_list<V>::user_list(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::user_list<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> user_list<V>::handle(fb::socket<character>& session, game_reqs::user_list<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    co_await ch->show_user_list();
    co_return true;
}

template class user_list<fb::protocol::CLIENT_VERSION::v550>;
template class user_list<fb::protocol::CLIENT_VERSION::v565>;
template class user_list<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
