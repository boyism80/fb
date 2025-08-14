#include <fb/game/handler/item_drop.h>
#include <fb/game/server.h>

using namespace fb::game::handler;

item_drop::item_drop(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::item_drop>(server)
{ }

async::task<bool> item_drop::handle(fb::socket<character>& session, fb::protocol::game::request::item_drop& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.drop(request.index, request.all ? -1 : 1);
    co_return true;
}
