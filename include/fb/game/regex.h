#ifndef __REGEX_H__
#define __REGEX_H__

#include <string>
#include <optional>
#include <fb/game/model.h>

namespace fb { namespace game { namespace regex {

bool                match_sell_message(const std::string& message, fb::game::item::model*& item, std::optional<uint16_t>& count);
bool                match_buy_message(const std::string& message, fb::game::item::model*& item, uint16_t& count);
bool                match_repair_message(const std::string& message, fb::game::item::model*& item, bool& all);
bool                match_deposit_money_message(const std::string& message, std::optional<uint32_t>& money);
bool                match_withdraw_money_message(const std::string& message, std::optional<uint32_t>& count);
bool                match_deposit_item_message(const std::string& message, fb::game::item::model*& item, std::optional<uint16_t>& count);
bool                match_withdraw_item_message(const std::string& message, fb::game::item::model*& item, std::optional<uint16_t>& count);

} } }

#endif