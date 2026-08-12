#include <fb/game/handler/protocol/dialog_list.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
dialog_list<V>::dialog_list(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::dialog_list<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> dialog_list<V>::handle(fb::socket<character>& session, game_reqs::dialog_list<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->dialog == nullptr)
        co_return true;

    auto lua   = ch->dialog;
    ch->dialog = nullptr;
    switch (request.type)
    {
    case fb::game::dialog::list_type::TEXT:
    case fb::game::dialog::list_type::TEXT_NO_MSG:
        lua->pushinteger(request.action);
        lua->resume(1);
        break;

    case fb::game::dialog::list_type::INPUT:
    case fb::game::dialog::list_type::INPUT_NO_MSG:
    case fb::game::dialog::list_type::INPUT_PASSWORD:
    case fb::game::dialog::list_type::INPUT_PASSWORD_NO_MSG:
    case fb::game::dialog::list_type::EMAIL:
        if (request.action == 0x02) // OK button
            lua->pushstring(request.message);
        else
            lua->pushinteger(request.action);

        lua->resume(1);
        break;

    case fb::game::dialog::list_type::LIST:
    case fb::game::dialog::list_type::LIST_NO_MSG:
        if (request.button == DIALOG_RESULT::NEXT)
            lua->pushinteger(request.index);
        else
            lua->pushnil();

        lua->pushinteger(static_cast<uint32_t>(request.button));
        lua->resume(2);
        break;

    default:
        lua->release();
        break;
    }

    co_return true;
}

template class dialog_list<fb::protocol::CLIENT_VERSION::v550>;
template class dialog_list<fb::protocol::CLIENT_VERSION::v565>;
template class dialog_list<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
