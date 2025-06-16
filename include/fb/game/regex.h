#ifndef __REGEX_H__
#define __REGEX_H__

#include <fb/model/model.h>

namespace fb { namespace game { namespace regex {

/**
 * @brief      Matches a sell message pattern and extracts item and count information.
 *
 *             Parses player input messages to detect item selling commands, extracting
 *             the target item and optional quantity. Used by merchant NPCs to process
 *             player sell requests through natural language commands.
 *
 * @param[in]  message  The player's input message to parse.
 * @param[in]  item     The item model to match against.
 * @param      count    Output parameter for the extracted quantity (optional).
 *
 * @return     True if the message matches a sell pattern, false otherwise.
 */
bool match_sell_message(const std::string& message, const fb::model::item* item, std::optional<uint16_t>& count);
/**
 * @brief      Matches a buy message pattern and extracts item and count information.
 *
 *             Parses player input messages to detect item purchasing commands, extracting
 *             the target item and quantity. Used by merchant NPCs to process player
 *             buy requests through natural language commands.
 *
 * @param[in]  message  The player's input message to parse.
 * @param[in]  item     The item model to match against.
 * @param      count    Output parameter for the extracted quantity.
 *
 * @return     True if the message matches a buy pattern, false otherwise.
 */
bool match_buy_message(const std::string& message, const fb::model::item* item, uint16_t& count);
/**
 * @brief      Matches a repair message pattern for item repair requests.
 *
 *             Parses player input messages to detect item repair commands, identifying
 *             the specific item to be repaired. Used by blacksmith NPCs to process
 *             equipment repair requests.
 *
 * @param[in]  message  The player's input message to parse.
 * @param[in]  item     The item model to match for repair.
 *
 * @return     True if the message matches a repair pattern, false otherwise.
 */
bool match_repair_message(const std::string& message, const fb::model::item* item);
/**
 * @brief      Matches a money deposit message pattern and extracts the amount.
 *
 *             Parses player input messages to detect bank deposit commands, extracting
 *             the amount of money to deposit. Used by banker NPCs to process deposit
 *             transactions through natural language commands.
 *
 * @param[in]  message  The player's input message to parse.
 * @param      money    Output parameter for the extracted deposit amount (optional).
 *
 * @return     True if the message matches a deposit pattern, false otherwise.
 */
bool match_deposit_money_message(const std::string& message, std::optional<uint32_t>& money);
/**
 * @brief      Matches a money withdrawal message pattern and extracts the amount.
 *
 *             Parses player input messages to detect bank withdrawal commands, extracting
 *             the amount of money to withdraw. Used by banker NPCs to process withdrawal
 *             transactions through natural language commands.
 *
 * @param[in]  message  The player's input message to parse.
 * @param      money    Output parameter for the extracted withdrawal amount (optional).
 *
 * @return     True if the message matches a withdrawal pattern, false otherwise.
 */
bool match_withdraw_money_message(const std::string& message, std::optional<uint32_t>& money);
/**
 * @brief      Matches an item storage message pattern and extracts item and count.
 *
 *             Parses player input messages to detect item storage commands for bank
 *             storage systems, extracting the target item and optional quantity.
 *
 * @param[in]  message  The player's input message to parse.
 * @param[in]  item     The item model to match for storage.
 * @param      count    Output parameter for the extracted quantity (optional).
 *
 * @return     True if the message matches a storage pattern, false otherwise.
 */
bool match_store_item_message(const std::string& message, const fb::model::item* item, std::optional<uint16_t>& count);
/**
 * @brief      Matches an item retrieval message pattern and extracts item and count.
 *
 *             Parses player input messages to detect item retrieval commands for bank
 *             storage systems, extracting the target item and optional quantity.
 *
 * @param[in]  message  The player's input message to parse.
 * @param[in]  item     The item model to match for retrieval.
 * @param      count    Output parameter for the extracted quantity (optional).
 *
 * @return     True if the message matches a retrieval pattern, false otherwise.
 */
bool match_retrieve_item_message(const std::string&       message,
                                 const fb::model::item*   item,
                                 std::optional<uint16_t>& count);

/**
 * @brief      Matches a sell list request message pattern.
 *
 *             Parses player input messages to detect requests for viewing the
 *             merchant's sell list or available items for purchase.
 *
 * @param[in]  message  The player's input message to parse.
 *
 * @return     True if the message matches a sell list request pattern, false otherwise.
 */
bool match_sell_list(const std::string& message);

/**
 * @brief      Matches a buy list request message pattern.
 *
 *             Parses player input messages to detect requests for viewing the
 *             merchant's buy list or items they will purchase from players.
 *
 * @param[in]  message  The player's input message to parse.
 *
 * @return     True if the message matches a buy list request pattern, false otherwise.
 */
bool match_buy_list(const std::string& message);

/**
 * @brief      Matches a sell price inquiry message pattern.
 *
 *             Parses player input messages to detect price inquiries for selling
 *             a specific item to a merchant, extracting the target item.
 *
 * @param[in]  message  The player's input message to parse.
 * @param[in]  item     The item model to match for price inquiry.
 *
 * @return     True if the message matches a sell price inquiry pattern, false otherwise.
 */
bool match_sell_price(const std::string& message, const fb::model::item* item);

/**
 * @brief      Matches a buy price inquiry message pattern.
 *
 *             Parses player input messages to detect price inquiries for buying
 *             a specific item from a merchant, extracting the target item.
 *
 * @param[in]  message  The player's input message to parse.
 * @param[in]  item     The item model to match for price inquiry.
 *
 * @return     True if the message matches a buy price inquiry pattern, false otherwise.
 */
bool match_buy_price(const std::string& message, const fb::model::item* item);

/**
 * @brief      Matches a deposited money confirmation message pattern.
 *
 *             Parses messages to detect confirmations of successful money deposits
 *             in banking systems, typically used for transaction verification.
 *
 * @param[in]  message  The message to parse for deposit confirmation.
 *
 * @return     True if the message matches a deposit confirmation pattern, false otherwise.
 */
bool match_deposited_money(const std::string& message);

/**
 * @brief      Matches a weapon rename message pattern and extracts the new name.
 *
 *             Parses player input messages to detect weapon renaming commands,
 *             extracting both the target weapon and the desired new name.
 *
 * @param[in]  message  The player's input message to parse.
 * @param[in]  item     The weapon item model to match for renaming.
 * @param      name     Output parameter for the extracted new weapon name.
 *
 * @return     True if the message matches a weapon rename pattern, false otherwise.
 */
bool match_rename_weapon(const std::string& message, const fb::model::item* item, std::string& name);

/**
 * @brief      Matches a stored item list request message pattern.
 *
 *             Parses player input messages to detect requests for viewing the
 *             list of items currently stored in bank storage.
 *
 * @param[in]  message  The player's input message to parse.
 *
 * @return     True if the message matches a stored item list request pattern, false otherwise.
 */
bool match_store_item_list(const std::string& message);

/**
 * @brief      Matches a stored item count inquiry message pattern.
 *
 *             Parses player input messages to detect inquiries about the quantity
 *             of a specific item currently stored in bank storage.
 *
 * @param[in]  message  The player's input message to parse.
 * @param[in]  item     The item model to match for count inquiry.
 *
 * @return     True if the message matches a stored item count inquiry pattern, false otherwise.
 */
bool match_store_item_count(const std::string& message, const fb::model::item* item);

}}} // namespace fb::game::regex

#endif