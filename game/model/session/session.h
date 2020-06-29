#ifndef __MMO_H__
#define __MMO_H__

#include <stdint.h>
#include <string>
#include <algorithm>
#include <optional>
#include <sstream>
#include <iostream>
#include <module/socket/socket.h>
#include <model/item/item.h>
#include <model/object/object.h>
#include <model/spell/spell.h>
#include <model/trade/trade.h>
#include <model/lua/lua.h>
#include <model/mob/mob.h>

namespace fb { namespace game {

class map;
class clan;
class group;
class items;
class listener;

class session : public life
{
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
    DECLARE_EXCEPTION(require_class_exception, message::exception::REQUIRE_CLASS)
    DECLARE_EXCEPTION(ghost_exception, message::exception::GHOST)
    DECLARE_EXCEPTION(ridding_exception, message::exception::RIDDING)
    DECLARE_EXCEPTION(no_conveyance_exception, message::exception::NO_CONVEYANCE)
    DECLARE_EXCEPTION(disguise_exception, message::exception::DISGUISE)
#pragma endregion

#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region friend
    friend group;
    friend clan;
#pragma endregion


#pragma region private field
private:
    fb::socket*                 _socket;
    listener*                   _listener;
    std::string                 _name;
    uint16_t                    _look;
    uint8_t                     _color;
    uint8_t                     _dress_color;

    defensive                   _defensive;
    uint32_t                    _base_hp, _base_mp;
    uint32_t                    _experience;

private:
    uint8_t                     _strength, _intelligence, _dexteritry;
    uint8_t                     _damage; // 공격수정
    uint8_t                     _hit; // 명중수정
    uint8_t                     _regenerative; // 재생력

    nation                      _nation;
    creature                    _creature;
    sex                         _sex;
    state                       _state;
    uint8_t                     _level;
    uint8_t                     _class, _promotion;
    uint32_t                    _money;
    std::optional<uint16_t>     _disguise;

    bool                        _options[0x0B+1];

    std::string                 _title;

    group*                      _group;
    clan*                       _clan;
#pragma endregion

#pragma region public field
public:
    fb::game::trade             trade;
    fb::game::items             items;
    legend_container            legends;
    lua::thread*                dialog_thread;
#pragma endregion


#pragma region hide parent method
private:
    using object::based;
#pragma endregion


#pragma region constructor / destructor
public:
    session(fb::socket* socket, listener* listener);
    ~session();
#pragma endregion


#pragma region override method
public:
    void                        send(const fb::ostream& stream, bool encrypt, bool wrap = true);
    object::types               type() const;
#pragma endregion


#pragma region middle man
    fb::cryptor&                crt();
    void                        crt(const fb::cryptor& crt);
    void                        crt(uint8_t enctype, const uint8_t* enckey);
    fb::istream&                in_stream();
#pragma endregion


#pragma region operator method
public:
    operator                    fb::socket& ();
#pragma endregion


#pragma region public method
public:
    void                        attack();
    void                        action(fb::game::action action, fb::game::duration duration, uint8_t sound = 0x00);
    uint32_t                    hp_down(uint32_t value, fb::game::object* from = nullptr, bool critical = false);

    const std::string&          name() const;
    void                        name(const std::string& value);

    uint16_t                    look() const;
    void                        look(uint16_t value);

    uint8_t                     color() const;
    void                        color(uint8_t value);

    uint8_t                     dress_color() const;
    void                        dress_color(uint8_t value);

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

    uint32_t                    random_damage(fb::game::life& life, bool& critical) const;

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

    void                        state_assert(fb::game::state flags) const;
    void                        state_assert(uint8_t flags) const;

    bool                        move(const point16_t& before);
    bool                        move(fb::game::direction direction, const point16_t& before);

    void                        ride(fb::game::mob& horse);
    void                        ride();
    void                        unride(fb::game::listener* listener);
    bool                        alive() const;
#pragma endregion


#pragma region make stream method
public:
    fb::ostream                 make_show_stream(bool light = false) const;
    fb::ostream                 make_id_stream() const;
    fb::ostream                 make_state_stream(fb::game::state_level level) const;
    fb::ostream                 make_appears_stream() const;
    fb::ostream                 make_position_stream() const;
    fb::ostream                 make_visual_stream(bool light) const;

    fb::ostream                 make_internal_info_stream() const;
    fb::ostream                 make_external_info_stream() const;
    fb::ostream                 make_option_stream() const;
    fb::ostream                 make_throw_item_stream(const item& item) const;
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

#pragma endregion
};

} }

#endif // !__MMO_H__
