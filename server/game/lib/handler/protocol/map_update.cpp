#include <fb/game/handler/protocol/map_update.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

map_update::map_update(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::map_update>(server)
{ }

async::task<bool> map_update::handle(fb::socket<character>& session, fb::protocol::game::request::map_update& request)
{
    auto ch = session.data();
    if (ch == nullptr)
        co_return true;

    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    ch->update_map(*map, request.begin, request.size, request.crc);
    co_return true;
}
