#ifndef __NPC_H__
#define __NPC_H__

#include <object.h>
#include <item.h>
#include <fb/encoding.h>
#include <format>
#include <optional>

namespace fb { namespace game {

/**
 * @brief      This class describes a character.
 */
class character;

/**
 * @brief      This class describes a npc.
 */
class npc : public object
{
public:
    using model_type = fb::model::npc;

public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    npc(fb::game::context& context, const fb::model::npc& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    npc(const npc& right);
    /**
     * @brief      Destroys the object.
     */
    ~npc();

public:
    /**
     * @brief      { function_description }
     *
     * @param      context  The context
     *
     * @return     { description_of_the_return_value }
     */
    npc* make(fb::game::context& context);
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     * @param[in]  item     The item
     * @param[in]  count    The count
     * @param[in]  bought   The bought
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool>
    buy(fb::game::character& session, const fb::model::item* item, std::optional<uint16_t> count, bool bought);
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     * @param[in]  item     The item
     * @param[in]  count    The count
     * @param[in]  sold     The sold
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool>
    sell(fb::game::character& session, const fb::model::item* item, uint16_t count, bool sold);
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     * @param[in]  item     The item
     * @param[in]  done     The done
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> repair(fb::game::character& session, const fb::model::item* item, bool done);
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     * @param[in]  money    The money
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> hold_money(fb::game::character& session, std::optional<uint32_t> money);
    /**
     * @brief      Returns a money.
     *
     * @param      session  The session
     * @param[in]  money    The money
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> return_money(fb::game::character& session, std::optional<uint32_t> money);
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     * @param[in]  item     The item
     * @param[in]  count    The count
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> hold_item(fb::game::character&    session,
                                              const fb::model::item*  item,
                                              std::optional<uint16_t> count);
    /**
     * @brief      Returns an item.
     *
     * @param      session  The session
     * @param[in]  item     The item
     * @param[in]  count    The count
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> return_item(fb::game::character&    session,
                                                const fb::model::item*  item,
                                                std::optional<uint16_t> count);
    /**
     * @brief      { function_description }
     */
    [[nodiscard]] async::task<void> sell_list();
    /**
     * @brief      { function_description }
     */
    [[nodiscard]] async::task<void> buy_list();
    /**
     * @brief      { function_description }
     *
     * @param[in]  item  The item
     */
    [[nodiscard]] async::task<void> sell_price(const fb::model::item* item);
    /**
     * @brief      { function_description }
     *
     * @param[in]  item  The item
     */
    [[nodiscard]] async::task<void> buy_price(const fb::model::item* item);
    /**
     * @brief      { function_description }
     *
     * @param[in]  session  The session
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> deposited_money(const fb::game::character& session);
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     * @param[in]  item     The item
     * @param[in]  name     The name
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> rename_weapon(fb::game::character&   session,
                                                  const fb::model::item* item,
                                                  const std::string&     name);
    /**
     * @brief      { function_description }
     *
     * @param[in]  session  The session
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> hold_item_list(const fb::game::character& session);
    /**
     * @brief      { function_description }
     *
     * @param[in]  session  The session
     * @param[in]  item     The item
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> hold_item_count(const fb::game::character& session, const fb::model::item* item);

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_input(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_menu(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_item(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_slot(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_sell(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buy(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_repair(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_repair_all(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_hold_money(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_hold_item(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_return_money(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_return_item(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_rename_weapon(lua_State* lua);
};

}} // namespace fb::game

#endif // !__NPC_H__
