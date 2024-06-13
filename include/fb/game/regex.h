#ifndef __REGEX_H__
#define __REGEX_H__

#include <string>
#include <optional>
#include <fb/game/old_model.h>

namespace fb { namespace game { namespace regex {

bool                match(const std::string& message, fb::game::container::regex::TYPE v);
bool                match_sell_message(const std::string& message, fb::game::item::model*& item, std::optional<uint16_t>& count);
bool                match_buy_message(const std::string& message, fb::game::item::model*& item, uint16_t& count);
bool                match_repair_message(const std::string& message, fb::game::item::model*& item);
bool                match_deposit_money_message(const std::string& message, std::optional<uint32_t>& money);
bool                match_withdraw_money_message(const std::string& message, std::optional<uint32_t>& money);
bool                match_deposit_item_message(const std::string& message, fb::game::item::model*& item, std::optional<uint16_t>& count);
bool                match_withdraw_item_message(const std::string& message, fb::game::item::model*& item, std::optional<uint16_t>& count);
bool                match_sell_list(const std::string& message);
bool                match_buy_list(const std::string& message);
bool                match_sell_price(const std::string& message, fb::game::item::model*& item);
bool                match_buy_price(const std::string& message, fb::game::item::model*& item);
bool                match_deposited_money(const std::string& message);
bool                match_rename_weapon(const std::string& message, fb::game::item::model*& item, std::string& name);
bool                match_hold_item_list(const std::string& message);
bool                match_hold_item_count(const std::string& message, fb::game::item::model*& item);

} } }

#endif