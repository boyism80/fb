#include <fb/game/handler/protocol/click.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

click::click(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::click>(server)
{ }

async::task<bool> click::handle(fb::socket<character>& session, game_reqs::click& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (request.oid == 0xFFFFFFFF) // Press F1
    {
        if (fb::model::const_value::script::F1_EVENT_SCRIPT == "")
            co_return true;

        if (fb::model::const_value::script::F1_EVENT_FUNC == "")
            co_return true;

        auto lua = fb::lua::new_context();
        if (lua == nullptr)
            co_return true;

#if defined DEBUG | defined _DEBUG
        lua->load(fb::model::const_value::script::F1_EVENT_SCRIPT);
#endif
        lua->func(fb::model::const_value::script::F1_EVENT_FUNC);
        lua->pushobject(ch);
        std::ignore = lua->call(1);
    }

    if (request.oid == 0xFFFFFFFE) // Preff F2
    {
        if (fb::model::const_value::script::F2_EVENT_SCRIPT == "")
            co_return true;

        if (fb::model::const_value::script::F2_EVENT_FUNC == "")
            co_return true;

        auto lua = fb::lua::new_context();
        if (lua == nullptr)
            co_return true;

#if defined DEBUG | defined _DEBUG
        lua->load(fb::model::const_value::script::F2_EVENT_SCRIPT);
#endif
        lua->func(fb::model::const_value::script::F2_EVENT_FUNC);
        lua->pushobject(ch);
        std::ignore = lua->call(1);
    }

    auto map = ch->map();
    auto you = map->objects[request.oid];
    if (you == nullptr)
        co_return true;

    switch (you->what())
    {
    case OBJECT_TYPE::CHARACTER:
    {
        ch->browse_ch(static_cast<character&>(*you));
    }
    break;

    case OBJECT_TYPE::MOB:
    {
        ch->send(game_resp::message(static_cast<mob&>(*you).name(), MESSAGE_TYPE::STATE));
    }
    break;

    case OBJECT_TYPE::NPC:
    {
        auto& model = static_cast<npc&>(*you).based<fb::model::npc>();
        if (model.script.empty())
            co_return true;

        auto lua = fb::lua::new_context();
        if (lua == nullptr)
            co_return true;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
        lua->load(model.script);
#endif
        lua->func(model.click);
        lua->pushobject(ch);
        lua->pushobject(static_cast<npc&>(*you));
        std::ignore = lua->call(2);
    }
    break;
    }

    co_return true;
}
