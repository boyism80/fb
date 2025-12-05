#include <fb/game/handler/protocol/friends.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

friends::friends(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::friends>(server)
{ }

async::task<bool> friends::handle(fb::socket<character>& session, fb::protocol::game::request::friends& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto&& resp = co_await this->server.http.post("internal", "/in-game/update-friends", UpdateFriends{ch->id, request.names});
    co_return true;
}
