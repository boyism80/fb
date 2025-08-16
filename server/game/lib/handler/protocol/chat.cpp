#include <fb/game/handler/protocol/chat.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

chat::chat(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::chat>(server)
{ }

async::task<bool> chat::handle(fb::socket<character>& session, fb::protocol::game::request::chat& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto weak = ch->weak_from_this_as<character>();
    auto map  = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->role() == ROLE::USER && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_TALK))
        co_return true;

    auto stop = false;
    auto lua  = fb::lua::new_context();
    if (lua != nullptr)
    {
#if defined DEBUG | defined _DEBUG
        lua->load("scripts/interaction.lua");
        lua->load("scripts/command.lua");
#endif
        lua->func("on_chat");
        lua->pushobject(ch);
        lua->pushstring(request.message);
        lua->pushboolean(request.shout);
        std::ignore = co_await lua->call(3, false);
        if (weak.expired())
        {
            lua->release();
            co_return true;
        }
        co_await ch->thread()->switching();

        stop = lua->toboolean(1);
        lua->release();
    }
    if (stop)
        co_return true;

    auto message = std::string{request.message};
    auto type    = request.shout ? CHAT_TYPE::SHOUT : CHAT_TYPE::NORMAL;
    ch->chat(message, type, true);

    auto npcs = std::vector<std::shared_ptr<fb::game::npc>>();
    if (type == CHAT_TYPE::SHOUT)
    {
        for (auto& [fd, obj] : ch->map()->objects)
        {
            if (obj->is(OBJECT_TYPE::NPC))
            {
                npcs.push_back(std::static_pointer_cast<fb::game::npc>(obj));
            }
        }
    }
    else
    {
        for (auto npc : ch->sight_in(OBJECT_TYPE::NPC))
        {
            npcs.push_back(std::static_pointer_cast<fb::game::npc>(npc));
        }
    }

    this->server.npc_interaction(*ch, request.message, npcs);

    co_return true;
}
