#include <fb/game/handler/protocol/loot.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

loot::loot(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::loot>(server)
{ }

async::task<bool> loot::handle(fb::socket<character>& session, fb::protocol::game::request::loot& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.loot(request.boost);
    co_return true;
}
