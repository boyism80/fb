#include <fb/game/handler/protocol/attack.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

attack::attack(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::attack>(server)
{ }

async::task<bool> attack::handle(fb::socket<character>& session, fb::protocol::game::request::attack& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    co_await ch->attack();
    co_return true;
}
