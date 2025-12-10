#include <fb/game/handler/protocol/friends.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

friends::friends(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::friends>(server)
{ }

async::task<bool> friends::handle(fb::socket<character>& session, game_reqs::friends& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto&& resp = co_await this->server.http.post("internal",
                                                  "/in-game/update-friends",
                                                  internal_reqs::UpdateFriends{ch->id, request.names});
    co_return true;
}
