#include <fb/game/handler/protocol/swap.h>
#include <fb/game/server.h>
#include <tuple>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
swap<V>::swap(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::swap<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> swap<V>::handle(fb::socket<character>& session, game_reqs::swap<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    switch (request.type)
    {
    case SWAP_TYPE::SPELL:
    {
        std::ignore = ch->spells.swap(request.src - 1, request.dst - 1);
        break;
    }

    case SWAP_TYPE::ITEM:
    {
        std::ignore = ch->items.swap(request.src - 1, request.dst - 1);
        break;
    }

    default:
        break;
    }
    co_return true;
}

template class swap<fb::protocol::CLIENT_VERSION::v550>;
template class swap<fb::protocol::CLIENT_VERSION::v565>;
template class swap<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
