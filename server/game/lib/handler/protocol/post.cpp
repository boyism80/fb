#include <fb/game/handler/protocol/post.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

post::post(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::post>(server)
{ }

async::task<bool> post::handle(fb::socket<character>& session, game_reqs::post& request)
{
    co_return true;
}
