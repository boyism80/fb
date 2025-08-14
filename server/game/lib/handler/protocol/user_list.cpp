#include <fb/game/handler/protocol/user_list.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

user_list::user_list(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::user_list>(server)
{ }

async::task<bool> user_list::handle(fb::socket<character>& session, fb::protocol::game::request::user_list& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    ch->show_user_list();
    co_return true;
}
