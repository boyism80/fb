#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <item.h>
#include <spell.h>
#include <trade.h>
#include <mob.h>
#include <dialog.h>
#include <npc.h>
#include <fb/socket.h>
#include <fb/string.h>
#include <stdint.h>
#include <algorithm>
#include <optional>
#include <sstream>
#include <iostream>
#include <fb/protocol/flatbuffer/protocol.h>

namespace fb { namespace game {

class map;
class clan;
class group;

class character : public life
{
    friend class group;
    friend class clan;

public:
    using fb::game::object::map;

class container : private std::vector<fb::game::character*>
{
public:
    using std::vector<fb::game::character*>::begin;
    using std::vector<fb::game::character*>::end;
    using std::vector<fb::game::character*>::size;
    using std::vector<fb::game::character*>::operator[];

public:
    container();
    container(const std::vector<fb::game::character*>& right);
    ~container();

public:
    container&                      push(fb::game::character& session);
    container&                      erase(fb::game::character& session);

public:
    fb::game::character*              find(const std::string& name);
    bool                            contains(const fb::game::character& session) const;

public:
    fb::game::character*              operator [] (const std::string& name);
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
    interface                       listener;

private:
    bool                            _init            = false;
    uint32_t                        _id              = 0xFFFFFFFF;
    fb::socket<character>&            _socket;
    bool                            _admin           = false;
    std::string                     _name;
    datetime             _last_login;
    uint16_t                        _look            = 0;
    uint8_t                         _color           = 0;
    std::optional<uint8_t>          _armor_color     = 0;

    defensive                       _defensive;
    uint32_t                        _base_hp         = 0;
    uint32_t                        _base_mp         = 0;
    uint32_t                        _experience      = 0;

private:
    uint8_t                         _strength        = 0;
    uint8_t                         _intelligence    = 0;
    uint8_t                         _dexteritry      = 0;
    uint8_t                         _damage          = 0; // 공격수정
    uint8_t                         _hit             = 0; // 명중수정
    uint8_t                         _regenerative    = 0; // 재생력

    NATION                          _nation          = NATION::GOGURYEO;
    CREATURE                        _creature        = CREATURE::DRAGON;
    SEX                             _sex             = SEX::ALL;
    STATE                           _state           = STATE::NORMAL;
    uint8_t                         _level           = 1;
    CLASS                           _class           = CLASS::NONE;
    uint8_t                         _promotion       = 0;
    uint32_t                        _money           = 0;
    std::optional<uint16_t>         _disguise        = 0;
    bool                            _options[0x0B+1] = {0,};
    uint32_t                        _deposited_money = 0;
    std::vector<item*>              _deposited_items;
    std::string                     _title;

    fb::game::group*                _group           = nullptr;
    fb::game::clan*                 _clan            = nullptr;

private:
    std::vector<std::function<async::task<bool>(const std::string&, const std::vector<fb::game::npc*>&)>> inline_interaction_funcs;

public:
    fb::game::trade                 trade            = fb::game::trade(*this);
    fb::game::items                 items            = fb::game::items(*this);
    fb::game::dialog                dialog           = fb::game::dialog(*this);
    legend_container                legends;

private:
    using object::based;

public:
    character(fb::game::context& context, fb::socket<fb::game::character>& socket);
    ~character();

protected:
    void                            on_hold() final;
    void                            on_update() final;
    uint32_t                        on_calculate_damage(bool critical) const final;
    void                            on_attack(fb::game::object* target) final;
    void                            on_kill(fb::game::life& you) final;
    void                            on_die(fb::game::object* from) final;

public:
    void                            send(const fb::ostream& stream, bool encrypt = true, bool wrap = true) final;
    void                            send(const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true) final;
    OBJECT_TYPE                     what() const final;
    async::task<bool>               map(fb::game::map* map, const point16_t& position, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) final;
    async::task<bool>               map(fb::game::map* map, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) final;

public:
    operator                        fb::socket<fb::game::character>& ();

public:
    bool                            inited() const;
    void                            init(bool value);
    uint32_t                        id() const;
    void                            id(uint32_t id);
    uint32_t                        fd();

    bool                            admin() const;
    void                            admin(bool value);

    bool                            transferring() const;

    void                            attack();
    void                            action(ACTION action, DURATION duration, uint8_t sound = 0x00);

    const std::string&              name() const final;
    void                            name(const std::string& value);

    const datetime&      last_login() const;
    void                            last_login(const datetime& value);

    uint16_t                        look() const final;
    void                            look(uint16_t value);

    uint8_t                         color() const final;
    void                            color(uint8_t value);

    std::optional<uint8_t>          armor_color() const;
    void                            armor_color(std::optional<uint8_t> value);
    uint8_t                         current_armor_color() const;

    std::optional<uint16_t>         disguise() const;
    void                            disguise(uint16_t value);
    void                            undisguise();

    uint32_t                        defensive_physical() const;
    void                            defensive_physical(uint8_t value);

    uint32_t                        defensive_magical() const;
    void                            defensive_magical(uint8_t value);

    void                            base_hp_up(uint32_t value);
    void                            base_mp_up(uint32_t value);

    void                            base_hp(uint32_t value);
    void                            base_mp(uint32_t value);

    uint32_t                        base_hp() const;
    uint32_t                        base_mp() const;

    NATION                          nation() const;
    bool                            nation(NATION value);

    CREATURE                        creature() const;
    bool                            creature(CREATURE value);

    uint8_t                         level() const;
    void                            level(uint8_t value);
    bool                            level_up();
    bool                            max_level() const;

    SEX                             sex() const;
    void                            sex(SEX value);

    STATE                           state() const;
    void                            state(STATE value);

    CLASS                           cls() const;
    void                            cls(CLASS value);

    uint8_t                         promotion() const;
    void                            promotion(uint8_t value);

    uint8_t                         strength() const;
    void                            strength(uint8_t value);
    void                            strength_up(uint8_t value);

    uint8_t                         intelligence() const;
    void                            intelligence(uint8_t value);
    void                            intelligence_up(uint8_t value);

    uint8_t                         dexteritry() const;
    void                            dexteritry(uint8_t value);
    void                            dexteritry_up(uint8_t value);

    uint32_t                        experience() const;
    void                            experience(uint32_t value);
    uint32_t                        experience_add(uint32_t value, bool notify = false);
    uint32_t                        experience_reduce(uint32_t value);
    uint32_t                        experience_remained() const;
    float                           experience_percent() const;

    uint32_t                        money() const;
    void                            money(uint32_t value);
    uint32_t                        money_add(uint32_t value);
    uint32_t                        money_reduce(uint32_t value);
    async::task<uint32_t>           money_drop(uint32_t value);

    uint32_t                        deposited_money() const;
    void                            deposited_money(uint32_t value);
    uint32_t                        deposit_money(uint32_t value);
    uint32_t                        withdraw_money(uint32_t value);

    bool                            deposit_item(fb::game::item& item);
    bool                            deposit_item(uint8_t index, uint16_t count);
    bool                            deposit_item(const std::string& name, uint16_t count);
    fb::game::item*                 deposited_item(const fb::model::item& item) const;
    const std::vector<item*>&       deposited_items() const;
    async::task<fb::game::item*>    withdraw_item(uint8_t index, uint16_t count);
    async::task<fb::game::item*>    withdraw_item(const std::string& name, uint16_t count);
    async::task<fb::game::item*>    withdraw_item(const fb::model::item& item, uint16_t count);

    uint32_t                        damage() const;
    void                            damage(uint8_t value);

    uint32_t                        hit() const;
    void                            hit(uint8_t value);

    uint32_t                        regenerative() const;
    void                            regenerative(uint8_t value);

    bool                            option(CUSTOM_SETTING key) const;
    void                            option(CUSTOM_SETTING key, bool value);
    bool                            option_toggle(CUSTOM_SETTING key);

    const std::string&              title() const;
    void                            title(const std::string& value);

    fb::game::group*                group() const;
    fb::game::clan*                 clan() const;

    void                            assert_state(STATE value) const;
    void                            assert_state(const std::vector<STATE>& values) const;

    bool                            move(const point16_t& before);
    bool                            move(DIRECTION direction, const point16_t& before);

    async::task<void>               ride(fb::game::mob& horse);
    async::task<void>               ride();
    async::task<void>               unride();
    bool                            alive() const;
    void                            refresh_map();
    bool                            condition(const std::vector<fb::model::dsl>& conditions) const final;
    void                            message(const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE);

private:
    async::task<bool>               inline_sell(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_buy(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_repair(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_deposit_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_withdraw_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_deposit_item(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_withdraw_item(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_sell_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_buy_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_sell_price(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_buy_price(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_show_deposited_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_rename_weapon(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_hold_item_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    async::task<bool>               inline_hold_item_count(const std::string& message, const std::vector<fb::game::npc*>& npcs);

public:
    async::task<bool>               inline_interaction(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    fb::protocol::db::Character     to_protocol() const;

public:
    static int                      builtin_look(lua_State* lua);
    static int                      builtin_color(lua_State* lua);
    static int                      builtin_money(lua_State* lua);
    static int                      builtin_exp(lua_State* lua);
    static int                      builtin_base_hp(lua_State* lua);
    static int                      builtin_base_mp(lua_State* lua);
    static int                      builtin_strength(lua_State* lua);
    static int                      builtin_dexterity(lua_State* lua);
    static int                      builtin_intelligence(lua_State* lua);
    static int                      builtin_item(lua_State* lua);
    static int                      builtin_items(lua_State* lua);
    static int                      builtin_item_drop(lua_State* lua);
    static int                      builtin_mkitem(lua_State* lua);
    static int                      builtin_rmitem(lua_State* lua);
    static int                      builtin_state(lua_State* lua);
    static int                      builtin_disguise(lua_State* lua);
    static int                      builtin_class(lua_State* lua);
    static int                      builtin_level(lua_State* lua);
    static int                      builtin_group(lua_State* lua);
    static int                      builtin_assert(lua_State* lua);
    static int                      builtin_admin(lua_State* lua);
    static int                      builtin_deposited_money(lua_State* lua);
    static int                      builtin_deposited_item(lua_State* lua);
    static int                      builtin_deposit_item(lua_State* lua);
    static int                      builtin_withdraw_item(lua_State* lua);
};


interface character::listener : public virtual fb::game::life::listener, 
    public virtual                  fb::game::dialog::listener,
    public virtual                  fb::game::trade::listener,
    public virtual                  fb::game::equipment::listener
{
public:
    virtual void                    on_hold(character& me) = 0;
    virtual void                    on_action(character& me, ACTION action, DURATION duration, uint8_t sound) = 0;
    virtual void                    on_updated(character& me, STATE_LEVEL level = STATE_LEVEL::LEVEL_MIN) = 0;
    virtual void                    on_money_changed(character& me, uint32_t value) = 0;
    virtual void                    on_notify(character& me, const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE) = 0;
    virtual void                    on_option(character& me, CUSTOM_SETTING option, bool enabled) = 0;
    virtual void                    on_level_up(character& me) = 0;
    virtual async::task<bool>       on_transfer(character& me, fb::game::map& map, const point16_t& position) = 0;
    virtual void                    on_item_get(character& me, const std::map<uint8_t, fb::game::item*>& items) = 0;
    virtual void                    on_item_changed(character& me, const std::map<uint8_t, fb::game::item*>& items) = 0;
    virtual void                    on_item_lost(character& me, const std::vector<uint8_t>& slots) = 0;
};

} }

#endif // !__MMO_H__
