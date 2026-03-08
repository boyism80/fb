#include <fb/game/handler/protocol/pong.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

async::task<bool> pong::handle(fb::socket<character>& session, game_reqs::pong& request)
{
    auto ch = session.data();
    if (ch == nullptr)
        co_return true;

    auto& state = ch->ping_state();
    if (request.token != state.token)
        co_return false;

    state.pong_received = true;
    co_return true;
}
