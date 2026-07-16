#include <fb/game/handler/protocol/item_active.h>
#include <fb/game/server.h>
#include <tuple>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

item_active::item_active(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_active>(server)
{ }

async::task<bool> item_active::handle(fb::socket<character>& session, game_reqs::item_active& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    std::ignore = co_await ch->items.active(request.index);
    co_return true;
}
