#include <fb/game/handler/protocol/update_screen.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

update_screen::update_screen(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::update_screen>(server)
{ }

async::task<bool> update_screen::handle(fb::socket<character>& session, game_reqs::update_screen& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    ch->update_position();
    co_return true;
}
