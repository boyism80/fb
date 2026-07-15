#include <fb/game/handler/protocol/item_inactive.h>
#include <fb/game/server.h>
#include <tuple>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

item_inactive::item_inactive(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_inactive>(server)
{ }

async::task<bool> item_inactive::handle(fb::socket<character>& session, game_reqs::item_inactive& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->items.free_size() == 0)
    {
        ch->message(_TEXT(MESSAGE_EXCEPTION_INVENTORY_OVERFLOW));
        co_return true;
    }

    std::ignore = co_await ch->items.inactive(request.parts);
    co_return true;
}
