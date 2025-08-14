#include <fb/game/handler/protocol/direction.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

direction::direction(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::direction>(server)
{ }

async::task<bool> direction::handle(fb::socket<character>& session, fb::protocol::game::request::direction& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->direction(request.value) == false)
        co_return false;

    co_return true;
}
