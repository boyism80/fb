#include <fb/game/handler/protocol/dialog.h>
#include <fb/game/server.h>

fb::game::handler::protocol::dialog::dialog(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::dialog>(server)
{ }

async::task<bool> fb::game::handler::protocol::dialog::handle(fb::socket<character>&               session,
                                                              fb::protocol::game::request::dialog& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->dialog == nullptr)
        co_return true;

    auto lua   = ch->dialog;
    ch->dialog = nullptr;
    switch (request.interaction)
    {
    case fb::game::dialog::interaction::NORMAL:
    {
        lua->pushinteger(request.action);
        lua->resume(1);
        break;
    }

    case fb::game::dialog::interaction::INPUT:
    {
        lua->pushstring(request.message);
        lua->resume(1);
        break;
    }

    case fb::game::dialog::interaction::INPUT_EX:
    {
        if (request.action == 0x02) // OK button
            lua->pushstring(request.message);
        else
            lua->pushinteger(request.action);

        lua->resume(1);
        break;
    }

    case fb::game::dialog::interaction::MENU:
    {
        lua->pushinteger(request.index);
        lua->resume(1);
        break;
    }

    case fb::game::dialog::interaction::LIST:
    {
        if (request.button == DIALOG_RESULT::NEXT)
            lua->pushinteger(request.index);
        else
            lua->pushnil();

        lua->pushinteger(static_cast<uint32_t>(request.button));
        lua->resume(2);
        break;
    }

    case fb::game::dialog::interaction::SLOT:
    {
        lua->pushinteger(request.index);
        lua->resume(1);
        break;
    }

    case fb::game::dialog::interaction::ITEM:
    {
        lua->pushstring(request.name);
        lua->resume(1);
        break;
    }

    default:
    {
        break;
    }
    }

    co_return true;
}
