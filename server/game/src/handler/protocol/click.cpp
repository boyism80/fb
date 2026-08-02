#include <fb/game/handler/protocol/click.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

click::click(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::click>(server)
{ }

async::task<bool> click::handle(fb::socket<character>& session, game_reqs::click& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    // Object-flag click (map object, F1/F2, dialog TOP): drop any waiting dialog first.
    // Real NPC then restarts via on_click; sentinel oid 0xFFFFFFFD just closes.
    if (request.flag == game_reqs::click::FLAG_OBJECT && ch->dialog != nullptr)
    {
        ch->dialog->release();
        ch->dialog = nullptr;
    }

    if (request.oid == 0xFFFFFFFF)
    {
        co_await handle_f1(ch);
        co_return true;
    }
    else if (request.oid == 0xFFFFFFFE)
    {
        co_await handle_f2(ch);
        co_return true;
    }
    else
    {
        co_await handle_object_click(ch, request);
        co_return true;
    }
}

async::task<void> click::handle_f1(character* ch)
{
    if (fb::model::const_value::script::F1_EVENT_SCRIPT == "")
        co_return;

    if (fb::model::const_value::script::F1_EVENT_FUNC == "")
        co_return;

    auto lua = this->server.lua.open(fb::model::const_value::script::F1_EVENT_SCRIPT,
                                     fb::model::const_value::script::F1_EVENT_FUNC);
    if (!lua)
        co_return;

    lua->pushobject(ch);
    std::ignore = lua->call(1);
}

async::task<void> click::handle_f2(character* ch)
{
    if (fb::model::const_value::script::F2_EVENT_SCRIPT == "")
        co_return;

    if (fb::model::const_value::script::F2_EVENT_FUNC == "")
        co_return;

    auto lua = this->server.lua.open(fb::model::const_value::script::F2_EVENT_SCRIPT,
                                     fb::model::const_value::script::F2_EVENT_FUNC);
    if (!lua)
        co_return;

    lua->pushobject(ch);
    std::ignore = lua->call(1);
}

async::task<void> click::handle_object_click(character* ch, game_reqs::click& request)
{
    auto map = ch->map();
    if (map == nullptr)
        co_return;

    auto you = map->objects[request.oid];
    if (you == nullptr)
        co_return;

    switch (you->what())
    {
    case OBJECT_TYPE::CHARACTER:
        ch->browse_ch(static_cast<character&>(*you));
        break;

    case OBJECT_TYPE::MOB:
        ch->message(static_cast<mob&>(*you).name(), MESSAGE_TYPE::STATE);
        break;

    case OBJECT_TYPE::NPC:
    {
        auto& model = static_cast<npc&>(*you).model();
        auto  path  = std::format("scripts/npc/{}.lua", model.id);
        auto  func  = "on_click";

        auto lua = this->server.lua.open(path, func);
        if (!lua)
            co_return;

        lua->pushobject(ch);
        lua->pushobject(static_cast<npc&>(*you));
        std::ignore = lua->call(2);
    }
    break;
    }
}
