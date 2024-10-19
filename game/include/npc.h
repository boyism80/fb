#ifndef __NPC_H__
#define __NPC_H__

#include <object.h>
#include <item.h>
#include <fb/encoding.h>
#include <format>
#include <optional>

namespace fb { namespace game {

class session;

class npc : public object
{
public:
    using model_type = fb::model::npc;

public:
    LUA_PROTOTYPE

public:
    npc(fb::game::context& context, const fb::model::npc& model);
    npc(const npc& right);
    ~npc();

public:
    npc*                        make(fb::game::context& context);
    bool                        buy(fb::game::session& session, const fb::model::item* item, std::optional<uint16_t> count, bool bought);
    bool                        sell(fb::game::session& session, const fb::model::item* item, uint16_t count, bool sold);
    bool                        repair(fb::game::session& session, const fb::model::item* item, bool done);
    bool                        hold_money(fb::game::session& session, std::optional<uint32_t> money);
    bool                        return_money(fb::game::session& session, std::optional<uint32_t> money);
    bool                        hold_item(fb::game::session& session, const fb::model::item* item, std::optional<uint16_t> count);
    async::task<bool>           return_item(fb::game::session& session, const fb::model::item* item, std::optional<uint16_t> count);
    void                        sell_list();
    void                        buy_list();
    void                        sell_price(const fb::model::item* item);
    void                        buy_price(const fb::model::item* item);
    bool                        deposited_money(const fb::game::session& session);
    bool                        rename_weapon(fb::game::session& session, const fb::model::item* item, const std::string& name);
    bool                        hold_item_list(const fb::game::session& session);
    bool                        hold_item_count(const fb::game::session& session, const fb::model::item* item);

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
    static int                  builtin_rename_weapon(lua_State* lua);
};

} }

#endif // !__NPC_H__
