#ifndef __MMO_H__
#define __MMO_H__

#include <stdint.h>
#include <string>
#include <algorithm>
#include <optional>
#include <sstream>
#include <iostream>
#include "module/socket/socket.h"
#include "model/item/item.h"
#include "model/object/object.h"
#include "model/spell/spell.h"
#include "model/trade/trade.h"
#include "model/mob/mob.h"
#include "model/npc/npc.h"
#include "module/string/string.h"
#include "model/lua/lua.h"

namespace fb { namespace game {

#pragma region forward declaration
class map;
class clan;
class group;
class session;
#pragma endregion

namespace lua {

class dialog
{
#pragma region forward nested declaration
public:
    interface listener;
#pragma endregion

private:
    session&                _owner;
    lua*                    _thread = nullptr;

public:
    dialog(fb::game::session& owner);
    ~dialog();

public:
    dialog&                 pushstring(const std::string& value);
    dialog&                 pushinteger(int value);
    dialog&                 pushnil();
    dialog&                 pushboolean(bool value);
    dialog&                 pushobject(const luable* object);
    dialog&                 pushobject(const luable& object);

public:
    dialog&                 from(const char* format, ...);
    dialog&                 func(const char* format, ...);
    bool                    resume(int argc);

public:
    void                    show(const object::master& object, const std::string& message, bool button_prev = false, bool button_next = true, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    show(const object& object, const std::string& message, bool button_prev = false, bool button_next = true, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    show(const npc::master& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    show(const npc& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    show(const npc::master& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    show(const npc& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    show(const npc::master& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    show(const npc& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    input(const npc::master& npc, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    input(const npc& npc, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    input(const npc::master& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    input(const npc& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
};

}


class session : public life
{
#pragma region friend
    friend class group;
    friend class clan;
#pragma endregion

public:
    using fb::game::object::map;

#pragma region container
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
#pragma endregion

#pragma region exception
public:
    DECLARE_EXCEPTION(require_class_exception, fb::game::message::exception::REQUIRE_CLASS)
    DECLARE_EXCEPTION(ghost_exception, fb::game::message::exception::GHOST)
    DECLARE_EXCEPTION(ridding_exception, fb::game::message::exception::RIDDING)
    DECLARE_EXCEPTION(no_conveyance_exception, fb::game::message::exception::NO_CONVEYANCE)
    DECLARE_EXCEPTION(disguise_exception, fb::game::message::exception::DISGUISE)
#pragma endregion

#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region forward nested declaration
public:
    interface                   listener;
#pragma endregion

#pragma region private field
private:
    uint32_t                    _id              = 0xFFFFFFFF;
    fb::socket<session>&        _socket;
    bool                        _transferring    = false;
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

    fb::game::nation            _nation          = nation::GOGURYEO;
    fb::game::creature          _creature        = creature::DRAGON;
    fb::game::sex               _sex             = fb::game::sex::BOTH;
    fb::game::state             _state           = state::NORMAL;
    uint8_t                     _level           = 1;
    uint8_t                     _class           = 0;
    uint8_t                     _promotion       = 0;
    uint32_t                    _money           = 0;
    std::optional<uint16_t>     _disguise        = 0;
    bool                        _options[0x0B+1] = {0,};

    std::string                 _title;

    fb::game::group*            _group           = nullptr;
    fb::game::clan*             _clan            = nullptr;
#pragma endregion

#pragma region public field
public:
    fb::game::trade             trade            = fb::game::trade(*this);
    fb::game::items             items            = fb::game::items(*this);
    lua::dialog                 dialog           = lua::dialog(*this);
    legend_container            legends;
#pragma endregion

#pragma region hide parent method
private:
    using object::based;
#pragma endregion

#pragma region constructor / destructor
public:
    session(fb::socket<fb::game::session>& socket, listener* listener);
    ~session();
#pragma endregion

#pragma region override method
protected:
    void                        handle_hold() final;
    void                        handle_switch_process(fb::game::map& map, const point16_t& position) final;
    void                        handle_warp() final;
    void                        handle_update() final;
    uint32_t                    handle_calculate_damage(bool critical) const final;
    void                        handle_attack(fb::game::object* target) final;
    void                        handle_hit(fb::game::life& target, uint32_t damage, bool critical) final;
    void                        handle_kill(fb::game::life& you) final;
    void                        handle_damaged(fb::game::object* from, uint32_t damage, bool critical) final;
    void                        handle_die(fb::game::object* from) final;

public:
    void                        handle_transfer(fb::game::map& map, const point16_t& position) final;

public:
    void                        send(const fb::ostream& stream, bool encrypt = true, bool wrap = true) final;
    void                        send(const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true) final;
    object::types               type() const final;
#pragma endregion

#pragma region operator
public:
    operator                    fb::socket<fb::game::session>& ();
#pragma endregion

#pragma region public method
public:
    uint32_t                    id() const;
    void                        id(uint32_t id);

    bool                        admin() const;
    void                        admin(bool value);

    bool                        transferring() const;

    void                        attack();
    void                        action(fb::game::action action, fb::game::duration duration, uint8_t sound = 0x00);

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

    fb::game::nation            nation() const;
    bool                        nation(fb::game::nation value);

    fb::game::creature          creature() const;
    bool                        creature(fb::game::creature value);

    uint8_t                     level() const;
    void                        level(uint8_t value);
    bool                        level_up();
    bool                        max_level() const;

    fb::game::sex               sex() const;
    void                        sex(fb::game::sex value);

    fb::game::state             state() const;
    void                        state(fb::game::state value);

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

    uint32_t                    damage() const;
    void                        damage(uint8_t value);

    uint32_t                    hit() const;
    void                        hit(uint8_t value);

    uint32_t                    regenerative() const;
    void                        regenerative(uint8_t value);

    bool                        option(options key) const;
    void                        option(options key, bool value);
    bool                        option_toggle(options key);

    const std::string&          title() const;
    void                        title(const std::string& value);

    fb::game::group*            group() const;
    fb::game::clan*             clan() const;

    void                        assert_state(fb::game::state value) const;
    void                        assert_state(const std::vector<fb::game::state>& values) const;

    bool                        move(const point16_t& before);
    bool                        move(fb::game::direction direction, const point16_t& before);

    void                        ride(fb::game::mob& horse);
    void                        ride();
    void                        unride(fb::game::mob::listener* listener);
    bool                        alive() const;
#pragma endregion

#pragma region built-in method
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

#pragma endregion
};


interface lua::dialog::listener
{
    virtual void                on_dialog(session& me, const object::master& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    virtual void                on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    virtual void                on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    virtual void                on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    virtual void                on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    virtual void                on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
};


interface session::listener : public virtual fb::game::life::listener, 
    public virtual              fb::game::lua::dialog::listener,
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
    virtual void                on_action(session& me, fb::game::action action, duration duration, uint8_t sound) = 0;
    virtual void                on_updated(session& me, state_level level = state_level::LEVEL_MIN) = 0;
    virtual void                on_money_changed(session& me, uint32_t value) = 0;
    virtual void                on_notify(session& me, const std::string& message, message::type type = message::type::STATE) = 0;
    virtual void                on_option(session& me, fb::game::options option, bool enabled) = 0;
    virtual void                on_level_up(session& me) = 0;
    virtual void                on_warp(fb::game::session& me) = 0;
    virtual void                on_transfer(session& me, fb::game::map& map, const point16_t& position) = 0;
    virtual void                on_item_get(session& me, const std::map<uint8_t, fb::game::item*>& items) = 0;
    virtual void                on_item_changed(session& me, const std::map<uint8_t, fb::game::item*>& items) = 0;
    virtual void                on_item_lost(session& me, const std::vector<uint8_t>& slots) = 0;
};

} }

#endif // !__MMO_H__
