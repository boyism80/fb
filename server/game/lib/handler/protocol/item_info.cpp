#include <fb/game/handler/protocol/item_info.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

item_info::item_info(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::item_info>(server)
{ }

async::task<bool> item_info::handle(fb::socket<character>& session, fb::protocol::game::request::item_info& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto item = ch->items[request.slot];
    if (item == nullptr)
        co_return false;

    ch->item_tooltip(*item, request.position);
    co_return true;
}
