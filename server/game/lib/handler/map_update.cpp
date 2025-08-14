#include <fb/game/handler/map_update.h>
#include <fb/game/server.h>

using namespace fb::game::handler;

map_update::map_update(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::map_update>(server)
{ }

async::task<bool> map_update::handle(fb::socket<character>& session, fb::protocol::game::request::map_update& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    ch->update_map(*map, request.position, request.size);
    co_return true;
}
