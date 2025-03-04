#include <fb/game/context.h>

using namespace fb::game;

async::task<bool> context::npc_interaction_sell(character&                         ch,
                                                const std::string&                 message,
                                                const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto count = std::optional<uint16_t>();
    auto name  = std::string();
    if (fb::model::const_value::regex::match_sell_message(message, name, count) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.buy.has_value() == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_buy_item");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (count.has_value())
            lua->pushinteger(count.value());
        else
            lua->pushnil();

        if (co_await lua->call(4, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_buy(character&                         ch,
                                               const std::string&                 message,
                                               const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name  = std::string();
    auto count = uint16_t(0);
    if (fb::model::const_value::regex::match_buy_message(message, name, count) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
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
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_repair(character&                         ch,
                                                  const std::string&                 message,
                                                  const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name = std::string();
    if (fb::model::const_value::regex::match_repair_message(message, name) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.repair)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_repair");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (co_await lua->call(3, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_deposit_money(character&                         ch,
                                                         const std::string&                 message,
                                                         const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto money = std::optional<uint32_t>();
    if (fb::model::const_value::regex::match_deposit_money_message(message, money) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_money == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_hold_money");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (money.has_value())
            lua->pushinteger(money.value());
        else
            lua->pushnil();
        if (co_await lua->call(3, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_withdraw_money(character&                         ch,
                                                          const std::string&                 message,
                                                          const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto money = std::optional<uint32_t>();
    if (fb::model::const_value::regex::match_withdraw_money_message(message, money) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_money == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_return_money");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (money.has_value())
            lua->pushinteger(money.value());
        else
            lua->pushnil();
        if (co_await lua->call(3, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_deposit_item(character&                         ch,
                                                        const std::string&                 message,
                                                        const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name  = std::string();
    auto count = std::optional<uint16_t>(0);
    if (fb::model::const_value::regex::match_deposit_item_message(message, name, count) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_item == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_hold_item");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (count.has_value())
            lua->pushinteger(count.value());
        else
            lua->pushnil();
        if (co_await lua->call(4, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_withdraw_item(character&                         ch,
                                                         const std::string&                 message,
                                                         const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name  = std::string();
    auto count = std::optional<uint16_t>(0);
    if (fb::model::const_value::regex::match_withdraw_item_message(message, name, count) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_item == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_return_item");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (count.has_value())
            lua->pushinteger(count.value());
        else
            lua->pushnil();
        if (co_await lua->call(4, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_sell_list(character&                         ch,
                                                     const std::string&                 message,
                                                     const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_sell_list(message) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.sell.size() == 0)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_sell_item_list");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (co_await lua->call(2, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_buy_list(character&                         ch,
                                                    const std::string&                 message,
                                                    const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_buy_list(message) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.buy.has_value() == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_buy_item_list");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (co_await lua->call(2, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_sell_price(character&                         ch,
                                                      const std::string&                 message,
                                                      const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name = std::string();
    if (fb::model::const_value::regex::match_sell_price(message, name) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.sell.size() == 0)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_sell_item_price");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (co_await lua->call(3, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_buy_price(character&                         ch,
                                                     const std::string&                 message,
                                                     const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name = std::string();
    if (fb::model::const_value::regex::match_buy_price(message, name) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.buy.has_value() == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_buy_item_price");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (co_await lua->call(3, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_show_deposited_money(character&                         ch,
                                                                const std::string&                 message,
                                                                const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_deposited_money(message) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_money == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_deposited_money");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (co_await lua->call(2, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_rename_weapon(character&                         ch,
                                                         const std::string&                 message,
                                                         const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    std::string model_name, custom_name;
    if (fb::model::const_value::regex::match_rename_weapon(message, model_name, custom_name) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.rename == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_rename_weapon");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(model_name);
        lua->pushstring(custom_name);
        if (co_await lua->call(4, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_hold_item_list(character&                         ch,
                                                          const std::string&                 message,
                                                          const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_hold_item_list(message) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_item == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_hold_item_list");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (co_await lua->call(2, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_hold_item_count(character&                         ch,
                                                           const std::string&                 message,
                                                           const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name = std::string();
    if (fb::model::const_value::regex::match_hold_item_count(message, name) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_item == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_hold_item_count");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (co_await lua->call(3, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_revive(character&                         ch,
                                                  const std::string&                 message,
                                                  const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto discourteous = false;
    if (fb::model::const_value::regex::match_revive(message, discourteous) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.revive == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_revive");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushboolean(discourteous);
        if (co_await lua->call(3, false) == false)
            co_return false;

        if (lua->pending())
            co_return true;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction_appreciate(character&                         ch,
                                                      const std::string&                 message,
                                                      const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_appreciate(message) == false)
        co_return false;

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.revive == false)
            continue;

#if defined DEBUG | defined _DEBUG
        lua->load("scripts/npc.lua");
#endif
        lua->func("npc_appreciate");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (co_await lua->call(2, false) == false)
            co_return false;

        if (lua->toboolean(1))
        {
            lua->release();
            co_return true;
        }
    }
    lua->release();
    co_return true;
}

async::task<bool> context::npc_interaction(character&                         ch,
                                           const std::string&                 message,
                                           const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (npcs.size() == 0)
        co_return false;

    for (auto& fn : this->_npc_interaction_funcs)
    {
        if (co_await fn(ch, message, npcs))
            co_return true;
    }

    co_return false;
}