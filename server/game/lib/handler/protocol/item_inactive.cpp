#include <fb/game/handler/protocol/item_inactive.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

item_inactive::item_inactive(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::item_inactive>(server)
{ }

async::task<bool> item_inactive::handle(fb::socket<character>&                      session,
                                        fb::protocol::game::request::item_inactive& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->items.free_size() == 0)
    {
        ch->message(_TEXT(MESSAGE_EXCEPTION_INVENTORY_OVERFLOW));
        co_return true;
    }

    ch->items.inactive(request.parts);
    co_return true;
}
