#include <fb/game/handler/protocol/popup_input_submit.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

async::task<bool> popup_input_submit::handle(fb::socket<character>& session, game_reqs::popup_input_submit& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    if (ch->dialog == nullptr)
        co_return true;

    auto lua    = ch->dialog;
    ch->dialog  = nullptr;
    std::ignore = request.param0;
    lua->pushstring(request.text);
    lua->resume(1);
    co_return true;
}
