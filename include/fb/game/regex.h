#ifndef __REGEX_H__
#define __REGEX_H__

#include <fb/model/model.h>

namespace fb { namespace game { namespace regex {

/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param[in]  item     The item
 * @param      count    The count
 *
 * @return     { description_of_the_return_value }
 */
bool match_sell_message(const std::string& message, const fb::model::item* item, std::optional<uint16_t>& count);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param[in]  item     The item
 * @param      count    The count
 *
 * @return     { description_of_the_return_value }
 */
bool match_buy_message(const std::string& message, const fb::model::item* item, uint16_t& count);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param[in]  item     The item
 *
 * @return     { description_of_the_return_value }
 */
bool match_repair_message(const std::string& message, const fb::model::item* item);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param      money    The money
 *
 * @return     { description_of_the_return_value }
 */
bool match_store_money_message(const std::string& message, std::optional<uint32_t>& money);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param      money    The money
 *
 * @return     { description_of_the_return_value }
 */
bool match_withdraw_money_message(const std::string& message, std::optional<uint32_t>& money);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param[in]  item     The item
 * @param      count    The count
 *
 * @return     { description_of_the_return_value }
 */
bool match_store_item_message(const std::string& message, const fb::model::item* item, std::optional<uint16_t>& count);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param[in]  item     The item
 * @param      count    The count
 *
 * @return     { description_of_the_return_value }
 */
bool match_retrieve_item_message(const std::string&       message,
                                 const fb::model::item*   item,
                                 std::optional<uint16_t>& count);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 *
 * @return     { description_of_the_return_value }
 */
bool match_sell_list(const std::string& message);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 *
 * @return     { description_of_the_return_value }
 */
bool match_buy_list(const std::string& message);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param[in]  item     The item
 *
 * @return     { description_of_the_return_value }
 */
bool match_sell_price(const std::string& message, const fb::model::item* item);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param[in]  item     The item
 *
 * @return     { description_of_the_return_value }
 */
bool match_buy_price(const std::string& message, const fb::model::item* item);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 *
 * @return     { description_of_the_return_value }
 */
bool match_deposited_money(const std::string& message);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param[in]  item     The item
 * @param      name     The name
 *
 * @return     { description_of_the_return_value }
 */
bool match_rename_weapon(const std::string& message, const fb::model::item* item, std::string& name);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 *
 * @return     { description_of_the_return_value }
 */
bool match_store_item_list(const std::string& message);
/**
 * @brief      { function_description }
 *
 * @param[in]  message  The message
 * @param[in]  item     The item
 *
 * @return     { description_of_the_return_value }
 */
bool match_store_item_count(const std::string& message, const fb::model::item* item);

}}} // namespace fb::game::regex

#endif