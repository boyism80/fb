#include <fb/game/handler/protocol/direction.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

direction::direction(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::direction>(server)
{ }

async::task<bool> direction::handle(fb::socket<character>& session, game_reqs::direction& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (co_await ch->direction(request.value) == false)
        co_return false;

    co_return true;
}
