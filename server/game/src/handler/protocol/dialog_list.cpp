#include <fb/game/handler/protocol/dialog_list.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

dialog_list::dialog_list(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::dialog_list>(server)
{ }

async::task<bool> dialog_list::handle(fb::socket<character>& session, game_reqs::dialog_list& request)
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
        lua->pushinteger(request.action);
        lua->resume(1);
        break;

    case fb::game::dialog::list_type::INPUT:
        if (request.action == 0x02) // OK button
            lua->pushstring(request.message);
        else
            lua->pushinteger(request.action);

        lua->resume(1);
        break;

    case fb::game::dialog::list_type::LIST:
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
