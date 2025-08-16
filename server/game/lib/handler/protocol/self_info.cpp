#include <fb/game/handler/protocol/self_info.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

self_info::self_info(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::self_info>(server)
{ }

async::task<bool> self_info::handle(fb::socket<character>& session, fb::protocol::game::request::self_info& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    ch->update_internal();
    ch->update_buff();
    co_return true;
}
