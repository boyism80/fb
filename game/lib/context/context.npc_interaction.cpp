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

    auto model = this->model.item.name2item(name);
    auto done  = false;
    for (auto npc : npcs)
    {
        if (npc->repair(ch, model, done))
            done = true;
    }

    return done;
}

bool context::npc_interaction_deposit_money(character&                         ch,
                                            const std::string&                 message,
                                            const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    auto money = std::optional<uint32_t>();
    if (fb::model::const_value::regex::match_deposit_money_message(message, money) == false)
        return false;

    for (auto npc : npcs)
    {
        if (npc->hold_money(ch, money))
            return true;
    }

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

    for (auto npc : npcs)
    {
        if (npc->return_money(ch, money))
            return true;
    }

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

    auto model = this->model.item.name2item(name);
    for (auto npc : npcs)
    {
        if (npc->hold_item(ch, model, count))
            return true;
    }

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

    auto model = this->model.item.name2item(name);
    for (auto npc : npcs)
    {
        if (npc->return_item(ch, model, count))
            return true;
    }

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

    for (auto npc : npcs)
    {
        if (npc->deposited_money(ch))
            return true;
    }

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

    auto model = this->model.item.name2item(model_name);
    for (auto npc : npcs)
    {
        if (npc->rename_weapon(ch, model, custom_name))
            return true;
    }

    return false;
}

bool context::npc_interaction_hold_item_list(character&                         ch,
                                             const std::string&                 message,
                                             const std::vector<fb::game::npc*>& npcs)
{
    ch.assert_thread();

    if (fb::model::const_value::regex::match_hold_item_list(message) == false)
        return false;

    for (auto npc : npcs)
    {
        if (npc->hold_item_list(ch))
            return true;
    }

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

    auto model = this->model.item.name2item(name);
    for (auto npc : npcs)
    {
        if (npc->hold_item_count(ch, model))
            return true;
    }

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