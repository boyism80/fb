#include <fb/game/handler/protocol/move_blocked.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

move_blocked::move_blocked(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::move_blocked>(server)
{ }

async::task<bool> move_blocked::handle(fb::socket<character>& session, game_reqs::move_blocked& request)
{
    co_return true;
}
