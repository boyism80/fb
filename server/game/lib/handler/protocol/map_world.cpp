#include <fb/game/handler/protocol/map_world.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

map_world::map_world(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::map_world>(server)
{ }

async::task<bool> map_world::handle(fb::socket<character>& session, fb::protocol::game::request::map_world& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto& world  = fb::model::table::world[request.value];
    auto& before = world[request.before];
    auto& after  = world[request.after];

    if (ch->map() == this->server.maps[after.map])
    {
        ch->update_map();
        ch->update_external(true);
    }
    else
    {
        std::ignore = co_await ch->map(this->server.maps[after.map], after.position);
    }
    co_return true;
}
