#include <fb/game/handler/protocol/dialog_list.h>

#include <fb/game/server.h>

#include <cstdint>

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
dialog_list<V>::dialog_list(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::dialog_list<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> dialog_list<V>::handle(fb::socket<character>& session, game_reqs::dialog_list<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    auto lua          = ch->dialog;
    auto maxlen       = ch->dialog_maxlen;
    ch->dialog        = nullptr;
    ch->dialog_maxlen = 0xFF;
    if (lua == nullptr)
        co_return true;

    auto reply = request;
    co_await lua->switching();

    switch (reply.type)
    {
    case fb::game::dialog::list_type::TEXT:
    case fb::game::dialog::list_type::TEXT_NO_MSG:
        lua->pushinteger(reply.action);
        lua->resume(1);
        break;

    case fb::game::dialog::list_type::INPUT:
    case fb::game::dialog::list_type::INPUT_NO_MSG:
    case fb::game::dialog::list_type::INPUT_PASSWORD:
    case fb::game::dialog::list_type::INPUT_PASSWORD_NO_MSG:
    case fb::game::dialog::list_type::EMAIL:
        // The client only displays maxlen, so a longer reply is refused as if the dialog were closed.
        if (reply.action == 0x02 && reply.message.size() <= maxlen) // OK button
            lua->pushstring(reply.message);
        else if (reply.action == 0x02)
            lua->pushinteger(static_cast<int>(DIALOG_RESULT::QUIT));
        else
            lua->pushinteger(reply.action);

        lua->resume(1);
        break;

    case fb::game::dialog::list_type::LIST:
    case fb::game::dialog::list_type::LIST_NO_MSG:
        if (reply.button == DIALOG_RESULT::NEXT)
            lua->pushinteger(reply.index);
        else
            lua->pushnil();

        lua->pushinteger(static_cast<uint32_t>(reply.button));
        lua->resume(2);
        break;

    default:
        lua->reject("unsupported dialog list type");
        break;
    }

    co_return true;
}

template class dialog_list<fb::protocol::CLIENT_VERSION::v550>;
template class dialog_list<fb::protocol::CLIENT_VERSION::v565>;
template class dialog_list<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
