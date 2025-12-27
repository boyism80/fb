#include <fb/game/handler/protocol/item_throws.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

item_throws::item_throws(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_throws>(server)
{ }

async::task<bool> item_throws::handle(fb::socket<character>& session, game_reqs::item_throws& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.throws(request.index, request.all);
    co_return true;
}
