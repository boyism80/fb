#ifndef __NPC_H__
#define __NPC_H__

#include <fb/game/object.h>
#include <fb/game/item.h>
#include <fb/core/encoding.h>
#include <fb/core/format.h>
#include <optional>

namespace fb { namespace game {

class session;

class npc : public object
{
public:
    LUA_PROTOTYPE

public:
    class model;

public:
    npc(fb::game::context& context, const model* model);
    npc(const npc& right);
    ~npc();

public:
    npc*                        make(fb::game::context& context);
    bool                        buy(fb::game::session& session, fb::game::item::model* item, std::optional<uint16_t> count, bool bought);
    bool                        sell(fb::game::session& session, fb::game::item::model* item, uint16_t count, bool sold);
    bool                        repair(fb::game::session& session, fb::game::item::model* item, bool done);
    bool                        hold_money(fb::game::session& session, std::optional<uint32_t> money);
    bool                        return_money(fb::game::session& session, std::optional<uint32_t> money);
    bool                        hold_item(fb::game::session& session, fb::game::item::model* item, std::optional<uint32_t> count);
    bool                        return_item(fb::game::session& session, fb::game::item::model* item, std::optional<uint32_t> count);
    void                        sell_list();
    void                        buy_list();
    void                        sell_price(const fb::game::item::model* item);
    void                        buy_price(const fb::game::item::model* item);
    bool                        deposited_money(const fb::game::session& session);
    bool                        rename_weapon(fb::game::session& session, const fb::game::item::model* item, const std::string& name);

public:
    static int                  builtin_input(lua_State* lua);
    static int                  builtin_menu(lua_State* lua);
    static int                  builtin_item(lua_State* lua);
    static int                  builtin_slot(lua_State* lua);
    static int                  builtin_sell(lua_State* lua);
    static int                  builtin_buy(lua_State* lua);
    static int                  builtin_repair(lua_State* lua);
    static int                  builtin_repair_all(lua_State* lua);
    static int                  builtin_hold_money(lua_State* lua);
    static int                  builtin_hold_item(lua_State* lua);
    static int                  builtin_return_money(lua_State* lua);
    static int                  builtin_return_item(lua_State* lua);
};

class npc::model : public object::model
{
public:
    LUA_PROTOTYPE

public:
    using pursuits = std::vector<std::pair<std::string, uint16_t>>;

public:
    interface listener;

    struct config : public fb::game::object::model::config
    {
    public:
        std::string                         script;
        std::map<std::string, uint16_t>     sell;
        std::optional<uint16_t>             buy;
        bool                                repair;
        bool                                hold_money, hold_item;
        bool                                rename;
    };

public:
    const std::string                       script;
    const std::map<std::string, uint16_t>   sell;
    const std::optional<uint16_t>           buy;
    const bool                              repair;
    const bool                              hold_money, hold_item;
    const bool                              rename;

public:
    friend class npc;

public:
    model(const fb::game::npc::model::config& config);
    ~model();

public:
    object::types               type() const { return object::types::NPC; }

public:
    bool                        contains_sell(const fb::game::item::model& item, uint32_t& price) const;
    bool                        contains_buy(const fb::game::item::model& item, uint32_t& price) const;

public:
    static int                  builtin_input(lua_State* lua);
    static int                  builtin_menu(lua_State* lua);
    static int                  builtin_item(lua_State* lua);
    static int                  builtin_slot(lua_State* lua);
    static int                  builtin_sell(lua_State* lua);
    static int                  builtin_buy(lua_State* lua);
    static int                  builtin_repair(lua_State* lua);
    static int                  builtin_repair_all(lua_State* lua);
    static int                  builtin_hold_money(lua_State* lua);
    static int                  builtin_hold_item(lua_State* lua);
    static int                  builtin_return_money(lua_State* lua);
    static int                  builtin_return_item(lua_State* lua);
};


interface npc::model::listener : public virtual fb::game::object::listener
{ };

} }

#endif // !__NPC_H__
