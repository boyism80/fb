#include <fb/game/handler/protocol/post.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

post::post(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::post>(server)
{ }

async::task<bool> post::handle(fb::socket<character>& session, fb::protocol::game::request::post& request)
{
    co_return true;
}
