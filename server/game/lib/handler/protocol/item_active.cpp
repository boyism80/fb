#include <fb/game/handler/protocol/item_active.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

item_active::item_active(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::item_active>(server)
{ }

async::task<bool> item_active::handle(fb::socket<character>& session, fb::protocol::game::request::item_active& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.active(request.index);
    co_return true;
}
