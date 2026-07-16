#include <fb/game/handler/protocol/swap.h>
#include <fb/game/server.h>
#include <tuple>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

swap::swap(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::swap>(server)
{ }

async::task<bool> swap::handle(fb::socket<character>& session, game_reqs::swap& request)
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
