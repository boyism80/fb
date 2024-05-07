#ifndef __MMO_H__
#define __MMO_H__

#include <stdint.h>
#include <string>
#include <algorithm>
#include <optional>
#include <sstream>
#include <iostream>
#include <fb/core/socket.h>
#include <fb/core/string.h>
#include <fb/game/item.h>
#include <fb/game/spell.h>
#include <fb/game/trade.h>
#include <fb/game/mob.h>
#include <fb/game/dialog.h>
#include <fb/game/npc.h>

namespace fb { namespace game {

class map;
class clan;
class group;

class session : public life
{
    friend class group;
    friend class clan;

public:
    using fb::game::object::map;

class container : private std::vector<fb::game::session*>
{
public:
    using std::vector<fb::game::session*>::begin;
    using std::vector<fb::game::session*>::end;
    using std::vector<fb::game::session*>::size;
    using std::vector<fb::game::session*>::operator[];

public:
    container();
    container(const std::vector<fb::game::session*>& right);
    ~container();

public:
    container&              push(fb::game::session& session);
    container&              erase(fb::game::session& session);

public:
    fb::game::session*      find(const std::string& name);
    bool                    contains(const fb::game::session& session) const;

public:
    fb::game::session*      operator [] (const std::string& name);
};

public:
    DECLARE_EXCEPTION(require_class_exception, fb::game::message::exception::REQUIRE_CLASS)
    DECLARE_EXCEPTION(ghost_exception, fb::game::message::exception::GHOST)
    DECLARE_EXCEPTION(ridding_exception, fb::game::message::exception::RIDDING)
    DECLARE_EXCEPTION(no_conveyance_exception, fb::game::message::exception::NO_CONVEYANCE)
    DECLARE_EXCEPTION(disguise_exception, fb::game::message::exception::DISGUISE)

public:
    LUA_PROTOTYPE

public:
    interface                   listener;

private:
    bool                        _init            = false;
    uint32_t                    _id              = 0xFFFFFFFF;
    fb::socket<session>&        _socket;
    bool                        _admin           = false;
    std::string                 _name;
    uint16_t                    _look            = 0;
    uint8_t                     _color           = 0;
    std::optional<uint8_t>      _armor_color     = 0;

    defensive                   _defensive;
    uint32_t                    _base_hp         = 0;
    uint32_t                    _base_mp         = 0;
    uint32_t                    _experience      = 0;

private:
    uint8_t                     _strength        = 0;
    uint8_t                     _intelligence    = 0;
    uint8_t                     _dexteritry      = 0;
    uint8_t                     _damage          = 0; // 공격수정
    uint8_t                     _hit             = 0; // 명중수정
    uint8_t                     _regenerative    = 0; // 재생력

    fb::game::NATION_TYPE       _nation          = NATION_TYPE::GOGURYEO;
    fb::game::CREATURE_TYPE     _creature        = CREATURE_TYPE::DRAGON;
    fb::game::SEX_TYPE          _sex             = fb::game::SEX_TYPE::BOTH;
    fb::game::STATE_TYPE        _state           = STATE_TYPE::NORMAL;
    uint8_t                     _level           = 1;
    uint8_t                     _class           = 0;
    uint8_t                     _promotion       = 0;
    uint32_t                    _money           = 0;
    std::optional<uint16_t>     _disguise        = 0;
    bool                        _options[0x0B+1] = {0,};
    uint32_t                    _deposited_money = 0;
    std::vector<item*>          _deposited_items;
    std::string                 _title;

    fb::game::group*            _group           = nullptr;
    fb::game::clan*             _clan            = nullptr;

private:
    std::vector<std::function<bool(const std::string&, const std::vector<fb::game::npc*>&)>> inline_interaction_funcs;

public:
    fb::game::trade             trade            = fb::game::trade(*this);
    fb::game::items             items            = fb::game::items(*this);
    fb::game::dialog            dialog           = fb::game::dialog(*this);
    legend_container            legends;

private:
    using object::based;

public:
    session(fb::socket<fb::game::session>& socket, fb::game::context& context);
    ~session();

protected:
    void                        on_hold() final;
    void                        on_update() final;
    uint32_t                    on_calculate_damage(bool critical) const final;
    void                        on_attack(fb::game::object* target) final;
    void                        on_hit(fb::game::life& target, uint32_t damage, bool critical) final;
    void                        on_kill(fb::game::life& you) final;
    void                        on_damaged(fb::game::object* from, uint32_t damage, bool critical) final;
    void                        on_die(fb::game::object* from) final;

public:
    void                        send(const fb::ostream& stream, bool encrypt = true, bool wrap = true) final;
    void                        send(const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true) final;
    object::types               type() const final;
    fb::awaitable<bool>         co_map(fb::game::map* map, const point16_t& position) final;
    fb::awaitable<bool>         co_map(fb::game::map* map) final;
    bool                        map(fb::game::map* map, const point16_t& position) final;
    bool                        map(fb::game::map* map) final;

public:
    operator                    fb::socket<fb::game::session>& ();

public:
    bool                        inited() const;
    void                        init(bool value);
    uint32_t                    id() const;
    void                        id(uint32_t id);
    uint32_t                    fd();

    bool                        admin() const;
    void                        admin(bool value);

    bool                        transferring() const;

    void                        attack();
    void                        action(fb::game::ACTION_TYPE action, fb::game::DURATION duration, uint8_t sound = 0x00);

    const std::string&          name() const final;
    void                        name(const std::string& value);

    uint16_t                    look() const final;
    void                        look(uint16_t value);

    uint8_t                     color() const final;
    void                        color(uint8_t value);

    std::optional<uint8_t>      armor_color() const;
    void                        armor_color(std::optional<uint8_t> value);
    uint8_t                     current_armor_color() const;

    std::optional<uint16_t>     disguise() const;
    void                        disguise(uint16_t value);
    void                        undisguise();

    uint32_t                    defensive_physical() const;
    void                        defensive_physical(uint8_t value);

    uint32_t                    defensive_magical() const;
    void                        defensive_magical(uint8_t value);

    void                        base_hp_up(uint32_t value);
    void                        base_mp_up(uint32_t value);

    void                        base_hp(uint32_t value);
    void                        base_mp(uint32_t value);

    uint32_t                    base_hp() const;
    uint32_t                    base_mp() const;

    fb::game::NATION_TYPE       nation() const;
    bool                        nation(fb::game::NATION_TYPE value);

    fb::game::CREATURE_TYPE     creature() const;
    bool                        creature(fb::game::CREATURE_TYPE value);

    uint8_t                     level() const;
    void                        level(uint8_t value);
    bool                        level_up();
    bool                        max_level() const;

    fb::game::SEX_TYPE          sex() const;
    void                        sex(fb::game::SEX_TYPE value);

    fb::game::STATE_TYPE        state() const;
    void                        state(fb::game::STATE_TYPE value);

    uint8_t                     cls() const;
    void                        cls(uint8_t value);

    uint8_t                     promotion() const;
    void                        promotion(uint8_t value);

    uint8_t                     strength() const;
    void                        strength(uint8_t value);
    void                        strength_up(uint8_t value);

    uint8_t                     intelligence() const;
    void                        intelligence(uint8_t value);
    void                        intelligence_up(uint8_t value);

    uint8_t                     dexteritry() const;
    void                        dexteritry(uint8_t value);
    void                        dexteritry_up(uint8_t value);

    uint32_t                    experience() const;
    void                        experience(uint32_t value);
    uint32_t                    experience_add(uint32_t value, bool notify = false);
    uint32_t                    experience_reduce(uint32_t value);
    uint32_t                    experience_remained() const;
    float                       experience_percent() const;

    uint32_t                    money() const;
    void                        money(uint32_t value);
    uint32_t                    money_add(uint32_t value);
    uint32_t                    money_reduce(uint32_t value);
    uint32_t                    money_drop(uint32_t value);

    uint32_t                    deposited_money() const;
    void                        deposited_money(uint32_t value);
    uint32_t                    deposit_money(uint32_t value);
    uint32_t                    withdraw_money(uint32_t value);

    bool                        deposit_item(uint8_t index, uint16_t count);
    bool                        deposit_item(const std::string& name, uint16_t count);
    fb::game::item*             deposited_item(const fb::game::item::model& item);
    const std::vector<item*>&   deposited_items() const;
    fb::game::item*             withdraw_item(uint8_t index, uint16_t count);
    fb::game::item*             withdraw_item(const std::string& name, uint16_t count);
    fb::game::item*             withdraw_item(const fb::game::item::model& item, uint16_t count);

    uint32_t                    damage() const;
    void                        damage(uint8_t value);

    uint32_t                    hit() const;
    void                        hit(uint8_t value);

    uint32_t                    regenerative() const;
    void                        regenerative(uint8_t value);

    bool                        option(OPTION key) const;
    void                        option(OPTION key, bool value);
    bool                        option_toggle(OPTION key);

    const std::string&          title() const;
    void                        title(const std::string& value);

    fb::game::group*            group() const;
    fb::game::clan*             clan() const;

    void                        assert_state(fb::game::STATE_TYPE value) const;
    void                        assert_state(const std::vector<fb::game::STATE_TYPE>& values) const;

    bool                        move(const point16_t& before);
    bool                        move(fb::game::DIRECTION_TYPE direction, const point16_t& before);

    void                        ride(fb::game::mob& horse);
    void                        ride();
    void                        unride();
    bool                        alive() const;
    void                        refresh_map();

private:
    bool                        inline_sell(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_buy(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_repair(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_deposit_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_withdraw_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_deposit_item(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_withdraw_item(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_sell_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_buy_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_sell_price(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_buy_price(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_show_deposited_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool                        inline_rename_weapon(const std::string& message, const std::vector<fb::game::npc*>& npcs);

public:
    bool                        inline_interaction(const std::string& message, const std::vector<fb::game::npc*>& npcs);

public:
    static int                  builtin_look(lua_State* lua);
    static int                  builtin_color(lua_State* lua);
    static int                  builtin_money(lua_State* lua);
    static int                  builtin_exp(lua_State* lua);
    static int                  builtin_base_hp(lua_State* lua);
    static int                  builtin_base_mp(lua_State* lua);
    static int                  builtin_strength(lua_State* lua);
    static int                  builtin_dexterity(lua_State* lua);
    static int                  builtin_intelligence(lua_State* lua);
    static int                  builtin_item(lua_State* lua);
    static int                  builtin_items(lua_State* lua);
    static int                  builtin_item_drop(lua_State* lua);
    static int                  builtin_mkitem(lua_State* lua);
    static int                  builtin_rmitem(lua_State* lua);
    static int                  builtin_state(lua_State* lua);
    static int                  builtin_disguise(lua_State* lua);
    static int                  builtin_class(lua_State* lua);
    static int                  builtin_level(lua_State* lua);
    static int                  builtin_group(lua_State* lua);
    static int                  builtin_assert(lua_State* lua);
    static int                  builtin_admin(lua_State* lua);
    static int                  builtin_deposited_money(lua_State* lua);
    static int                  builtin_deposited_item(lua_State* lua);
    static int                  builtin_deposit_item(lua_State* lua);
    static int                  builtin_withdraw_item(lua_State* lua);
};


interface session::listener : public virtual fb::game::life::listener, 
    public virtual              fb::game::dialog::listener,
    public virtual              fb::game::trade::listener,
    public virtual              fb::game::equipment::listener
{
public:
    virtual void                on_attack(session& me, object* you) = 0;
    virtual void                on_hit(session& me, life& you, uint32_t damage, bool critical) = 0;
    virtual void                on_kill(session& me, life& you) = 0;
    virtual void                on_damaged(session& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void                on_hold(session& me) = 0;
    virtual void                on_die(session& me, object* you) = 0;
    virtual void                on_action(session& me, fb::game::ACTION_TYPE action, fb::game::DURATION duration, uint8_t sound) = 0;
    virtual void                on_updated(session& me, STATE_LEVEL level = STATE_LEVEL::LEVEL_MIN) = 0;
    virtual void                on_money_changed(session& me, uint32_t value) = 0;
    virtual void                on_notify(session& me, const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE) = 0;
    virtual void                on_option(session& me, fb::game::OPTION option, bool enabled) = 0;
    virtual void                on_level_up(session& me) = 0;
    virtual void                on_transfer(session& me, fb::game::map& map, const point16_t& position, fb::awaitable<bool>* awaitable = nullptr) = 0;
    virtual void                on_item_get(session& me, const std::map<uint8_t, fb::game::item*>& items) = 0;
    virtual void                on_item_changed(session& me, const std::map<uint8_t, fb::game::item*>& items) = 0;
    virtual void                on_item_lost(session& me, const std::vector<uint8_t>& slots) = 0;
};

} }

#endif // !__MMO_H__
