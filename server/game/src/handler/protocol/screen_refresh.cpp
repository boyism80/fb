#include <fb/game/handler/protocol/screen_refresh.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

screen_refresh::screen_refresh(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::screen_refresh>(server)
{ }

async::task<bool> screen_refresh::handle(fb::socket<character>& session, game_reqs::screen_refresh& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    co_await ch->screen_refresh();
    co_return true;
}
