#include <fb/game/handler/protocol/friends.h>
#include <fb/game/server.h>

namespace game_reqs     = fb::protocol::game::request;
namespace internal_reqs = fb::protocol::internal::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
friends<V>::friends(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::friends<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> friends<V>::handle(fb::socket<character>& session, game_reqs::friends<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post("internal",
                                                  "/in-game/update-friends",
                                                  internal_reqs::UpdateFriends{world, ch->id, request.names});

    co_return true;
}

template class friends<fb::protocol::CLIENT_VERSION::v550>;
template class friends<fb::protocol::CLIENT_VERSION::v565>;
template class friends<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
