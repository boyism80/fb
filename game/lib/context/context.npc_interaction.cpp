#include <fb/game/context.h>

using namespace fb::game;

bool context::npc_interaction_sell(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto count = std::optional<uint16_t>();
    auto name  = std::string();
    if (fb::model::const_value::regex::match_sell_message(message, name, count) == false)
        return false;

    auto model  = this->model.item.name2item(name);
    auto bought = false;
    for (auto npc : npcs)
    {
        if (npc->buy(ch, model, count, bought))
            bought = true;
    }

    return bought;
}

bool context::npc_interaction_buy(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name  = std::string();
    auto count = uint16_t(0);
    if (fb::model::const_value::regex::match_buy_message(message, name, count) == false)
        return false;

    auto model = this->model.item.name2item(name);
    auto sold  = false;
    for (auto npc : npcs)
    {
        if (npc->sell(ch, model, count, sold))
            sold = true;
    }

    return sold;
}

bool context::npc_interaction_repair(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name = std::string();
    if (fb::model::const_value::regex::match_repair_message(message, name) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_money == false)
            continue;

        lua->func("npc_repair");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (lua->resume(3, false) == false)
            return false;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction_deposit_money(character&                         ch,
                                            const std::string&                 message,
                                            const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto money = std::optional<uint32_t>();
    if (fb::model::const_value::regex::match_deposit_money_message(message, money) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_money == false)
            continue;

        lua->func("npc_hold_money");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (money.has_value())
            lua->pushinteger(money.value());
        else
            lua->pushnil();
        if (lua->resume(3, false) == false)
            return false;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction_withdraw_money(character&                         ch,
                                             const std::string&                 message,
                                             const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto money = std::optional<uint32_t>();
    if (fb::model::const_value::regex::match_withdraw_money_message(message, money) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_money == false)
            continue;

        lua->func("npc_return_money");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (money.has_value())
            lua->pushinteger(money.value());
        else
            lua->pushnil();
        if (lua->resume(3, false) == false)
            return false;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction_deposit_item(character&                         ch,
                                           const std::string&                 message,
                                           const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name  = std::string();
    auto count = std::optional<uint16_t>(0);
    if (fb::model::const_value::regex::match_deposit_item_message(message, name, count) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_item == false)
            continue;

        lua->func("npc_hold_item");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (count.has_value())
            lua->pushinteger(count.value());
        else
            lua->pushnil();
        if (lua->resume(4, false) == false)
            return false;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction_withdraw_item(character&                         ch,
                                            const std::string&                 message,
                                            const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name  = std::string();
    auto count = std::optional<uint16_t>(0);
    if (fb::model::const_value::regex::match_withdraw_item_message(message, name, count) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_item == false)
            continue;

        lua->func("npc_return_item");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (count.has_value())
            lua->pushinteger(count.value());
        else
            lua->pushnil();
        if (lua->resume(4, false) == false)
            return false;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction_sell_list(character&                         ch,
                                        const std::string&                 message,
                                        const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_sell_list(message) == false)
        return false;

    for (auto npc : npcs)
    {
        npc->sell_list();
    }

    return true;
}

bool context::npc_interaction_buy_list(character&                         ch,
                                       const std::string&                 message,
                                       const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_buy_list(message) == false)
        return false;

    for (auto npc : npcs)
    {
        npc->buy_list();
    }

    return true;
}

bool context::npc_interaction_sell_price(character&                         ch,
                                         const std::string&                 message,
                                         const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name = std::string();
    if (fb::model::const_value::regex::match_sell_price(message, name) == false)
        return false;

    auto model = this->model.item.name2item(name);
    for (auto npc : npcs)
    {
        npc->sell_price(model);
    }

    return true;
}

bool context::npc_interaction_buy_price(character&                         ch,
                                        const std::string&                 message,
                                        const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name = std::string();
    if (fb::model::const_value::regex::match_buy_price(message, name) == false)
        return false;

    auto model = this->model.item.name2item(name);
    for (auto npc : npcs)
    {
        npc->buy_price(model);
    }

    return true;
}

bool context::npc_interaction_show_deposited_money(character&                         ch,
                                                   const std::string&                 message,
                                                   const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_deposited_money(message) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_money == false)
            continue;

        lua->func("npc_deposited_money");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (lua->resume(2, false) == false)
            return false;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction_rename_weapon(character&                         ch,
                                            const std::string&                 message,
                                            const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    std::string model_name, custom_name;
    if (fb::model::const_value::regex::match_rename_weapon(message, model_name, custom_name) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.rename == false)
            continue;

        lua->func("npc_rename_weapon");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(model_name);
        lua->pushstring(custom_name);
        if (lua->resume(4, false) == false)
            return false;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction_hold_item_list(character&                         ch,
                                             const std::string&                 message,
                                             const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_hold_item_list(message) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_item == false)
            continue;

        lua->func("npc_hold_item_list");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (lua->resume(2, false) == false)
            return false;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction_hold_item_count(character&                         ch,
                                              const std::string&                 message,
                                              const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto name = std::string();
    if (fb::model::const_value::regex::match_hold_item_count(message, name) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.hold_item == false)
            continue;

        lua->func("npc_hold_item_count");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushstring(name);
        if (lua->resume(3, false) == false)
            return false;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction_revive(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto discourteous = false;
    if (fb::model::const_value::regex::match_revive(message, discourteous) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.revive == false)
            continue;

        lua->func("npc_revive");
        lua->pushobject(ch);
        lua->pushobject(npc);
        lua->pushboolean(discourteous);
        if (lua->resume(3, false) == false)
            return false;

        if (lua->pending())
            return true;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction_appreciate(character&                         ch,
                                         const std::string&                 message,
                                         const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_appreciate(message) == false)
        return false;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/npc.lua");
#endif

    auto lua = fb::lua::new_context();
    for (auto npc : npcs)
    {
        auto& model = npc->based<fb::model::npc>();
        if (model.revive == false)
            continue;

        lua->func("npc_appreciate");
        lua->pushobject(ch);
        lua->pushobject(npc);
        if (lua->resume(2, false) == false)
            return false;

        if (lua->toboolean(1))
        {
            lua->release();
            return true;
        }
    }
    lua->release();
    return false;
}

bool context::npc_interaction(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (npcs.size() == 0)
        return false;

    for (auto& fn : this->_npc_interaction_funcs)
    {
        if (fn(ch, message, npcs))
            return true;
    }

    return false;
}