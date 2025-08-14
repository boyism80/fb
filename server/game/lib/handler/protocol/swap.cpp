#include <fb/game/handler/protocol/swap.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

swap::swap(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::swap>(server)
{ }

async::task<bool> swap::handle(fb::socket<character>& session, fb::protocol::game::request::swap& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    switch (request.type)
    {
    case SWAP_TYPE::SPELL:
    {
        ch->spells.swap(request.src - 1, request.dst - 1);
        break;
    }

    case SWAP_TYPE::ITEM:
    {
        ch->items.swap(request.src - 1, request.dst - 1);
        break;
    }

    default:
        break;
    }
    co_return true;
}
