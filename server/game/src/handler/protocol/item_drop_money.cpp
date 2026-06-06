#include <fb/game/handler/protocol/item_drop_money.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

item_drop_money::item_drop_money(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_drop_money>(server)
{ }

async::task<bool> item_drop_money::handle(fb::socket<character>& session, game_reqs::item_drop_money& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return false;

    auto chunk = std::min(ch->money(), request.chunk);

    ch->money_drop(chunk);
    co_return true;
}
