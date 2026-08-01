#include <fb/game/handler/protocol/dialog.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

dialog::dialog(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::dialog>(server)
{ }

async::task<bool> dialog::handle(fb::socket<character>& session, game_reqs::dialog& request)
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
    case fb::game::dialog::type::INPUT:
    case fb::game::dialog::type::INPUT_NO_EXT:
        lua->pushstring(request.message);
        lua->resume(1);
        break;

    case fb::game::dialog::type::MENU:
    case fb::game::dialog::type::MENU_NO_EXT:
        lua->pushinteger(request.index);
        lua->resume(1);
        break;

    case fb::game::dialog::type::SLOT:
    case fb::game::dialog::type::SPELL:
        lua->pushinteger(request.index);
        lua->resume(1);
        break;

    case fb::game::dialog::type::ITEM:
    case fb::game::dialog::type::PURSUIT:
    case fb::game::dialog::type::DUAL_FIELD:
        lua->pushstring(request.name);
        lua->resume(1);
        break;

    default:
        lua->release();
        break;
    }

    co_return true;
}
