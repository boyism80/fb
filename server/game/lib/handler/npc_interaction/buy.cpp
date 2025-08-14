#include <fb/game/handler/npc_interaction/buy.h>
#include <fb/game/server.h>
#include <fb/model/model.h>

using namespace fb::game::handler::npc_interaction;

async::task<void> buy::handle(character_type& ch, const std::string& message, const npc_list_type& npcs)
{
    ch.assert_thread();
    auto weak  = ch.weak_from_this();
    auto name  = std::string();
    auto count = uint16_t(0);
    if (fb::model::const_value::regex::match_buy_message(message, name, count) == false)
        co_return;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        co_return;

    for (auto& npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.sell.size() == 0)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_sell_item");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        lua->pushinteger(count);
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