#include <fb/game/handler/protocol/user_info_submit.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

async::task<bool> user_info_submit::handle(fb::socket<character>& session, game_reqs::user_info_submit& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    // Legacy NHN profile submit — no persistence in FB private server.
    std::ignore = request;
    co_return true;
}
