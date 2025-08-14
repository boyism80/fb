#include <fb/game/handler/npc_interaction/withdraw_money.h>
#include <fb/game/server.h>
#include <fb/model/model.h>

using namespace fb::game::handler::npc_interaction;

async::task<void> withdraw_money::handle(character_type& ch, const std::string& message, const npc_list_type& npcs)
{
    ch.assert_thread();
    auto weak  = ch.weak_from_this();
    auto money = std::optional<uint32_t>();
    if (fb::model::const_value::regex::match_withdraw_money_message(message, money) == false)
        co_return;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        co_return;

    for (auto& npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (ENUM_IN(model.interaction, NPC_INTERACTION::DEPOSIT_MONEY) == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_withdraw_money");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (money.has_value())
            lua->pushinteger(money.value());
        else
            lua->pushnil();
        if (co_await lua->call(3, false) == false)
            continue;

        if (weak.expired())
            break;

        co_await this->server.threads.switching(weak);

        if (lua->toboolean(1))
            break;
    }

    lua->release();
}