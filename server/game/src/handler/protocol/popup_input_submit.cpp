#include <fb/game/handler/protocol/popup_input_submit.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> popup_input_submit<V>::handle(fb::socket<character>&            session,
                                                game_reqs::popup_input_submit<V>& request)
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

template class popup_input_submit<fb::protocol::CLIENT_VERSION::v550>;
template class popup_input_submit<fb::protocol::CLIENT_VERSION::v565>;
template class popup_input_submit<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
