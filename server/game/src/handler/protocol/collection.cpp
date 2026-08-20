#include <fb/game/handler/protocol/collection.h>
#include <fb/game/protocol/collection.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> collection<V>::handle(fb::socket<character>& session, game_reqs::collection<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
    {
        if (request.action == 0)
            ch->listener.on_collection_list(*ch);
        else if (request.action == 1 && this->server.meta.group(request.slot) != nullptr)
            ch->listener.on_collection_dialog(*ch, request.slot);
    }
    co_return true;
}

template class collection<fb::protocol::CLIENT_VERSION::v550>;
template class collection<fb::protocol::CLIENT_VERSION::v565>;
template class collection<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
