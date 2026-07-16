#include <fb/game/handler/protocol/loot.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

loot::loot(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::loot>(server)
{ }

async::task<bool> loot::handle(fb::socket<character>& session, game_reqs::loot& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    co_await ch->items.loot(request.boost);
    co_return true;
}
