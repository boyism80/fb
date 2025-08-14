#include <fb/game/handler/npc_interaction/retrieve_item.h>
#include <fb/game/server.h>
#include <fb/model/model.h>

using namespace fb::game::handler::npc_interaction;

async::task<void> retrieve_item::handle(character_type& ch, const std::string& message, const npc_list_type& npcs)
{
    ch.assert_thread();
    auto weak  = ch.weak_from_this();
    auto name  = std::string();
    auto count = std::optional<uint16_t>(0);
    if (fb::model::const_value::regex::match_retrieve_item_message(message, name, count) == false)
        co_return;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        co_return;

    for (auto& npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (ENUM_IN(model.interaction, NPC_INTERACTION::STORE_ITEM) == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_retrieve_item");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (count.has_value())
            lua->pushinteger(count.value());
        else
            lua->pushnil();
        if (co_await lua->call(4, false) == false)
            continue;

        if (weak.expired())
            break;

        co_await this->server.threads.switching(weak);

        if (lua->toboolean(1))
            break;
    }

    lua->release();
}