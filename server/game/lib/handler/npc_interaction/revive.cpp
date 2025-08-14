#include <fb/game/handler/npc_interaction/revive.h>
#include <fb/game/server.h>
#include <fb/model/model.h>

using namespace fb::game::handler::npc_interaction;

async::task<void> revive::handle(character_type& ch, const std::string& message, const npc_list_type& npcs)
{
    ch.assert_thread();
    auto weak         = ch.weak_from_this();
    auto discourteous = false;
    if (fb::model::const_value::regex::match_revive(message, discourteous) == false)
        co_return;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        co_return;

    for (auto& npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (ENUM_IN(model.interaction, NPC_INTERACTION::REVIVE) == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_revive");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushboolean(discourteous);
        if (co_await lua->call(3, false) == false)
            continue;

        if (weak.expired())
            break;

        co_await this->server.threads.switching(weak);

        if (lua->pending())
            break;

        if (lua->toboolean(1))
            break;
    }

    lua->release();
}