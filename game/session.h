#ifndef __MMO_H__
#define __MMO_H__

#include <stdint.h>
#include <string>
#include "socket.h"
#include "acceptor.h"
#include <algorithm>
#include "item.h"
#include "object.h"
#include "spell.h"
#include "trade.h"
#include "group.h"
#include "lua.h"

namespace fb { namespace game {

class map;

class session : public fb_session, public life
{
public:
    DECLARE_EXCEPTION(require_class_exception, message::exception::REQUIRE_CLASS)
    DECLARE_EXCEPTION(ghost_exception, message::exception::GHOST)
    DECLARE_EXCEPTION(ridding_exception, message::exception::RIDDING)
    DECLARE_EXCEPTION(no_conveyance_exception, message::exception::NO_CONVEYANCE)
    DECLARE_EXCEPTION(disguise_exception, message::exception::DISGUISE)

public:
	DECLARE_LUA_PROTOTYPE

private:
	std::string             _name;
	uint16_t                _look;
	uint8_t                 _color;

	defensive				_defensive;
	uint32_t                _base_hp, _base_mp;
	uint32_t                _experience;

private:
    uint8_t                 _strength, _intelligence, _dexteritry;
    uint8_t                 _damage; // 공격수정
    uint8_t                 _hit; // 명중수정
    uint8_t                 _regenerative; // 재생력

    nation					_nation;
    creature				_creature;
    sex						_sex;
    state					_state;
    uint8_t                 _level;
    uint8_t                 _class, _promotion;
    uint32_t                _money;

    bool                    _options[0x0B+1];
    std::vector<legend>     _legends;

    std::string             _title;

    group*                  _group;

public:
	fb::game::trade         trade;
	fb::game::items			items;
	fb::game::spells		spells;
	lua::thread*			dialog_thread;

private:
	using object::based;

public:
    session(SOCKET socket);
    ~session();

public:
    const std::string&      name() const;
    void                    name(const std::string& value);

    uint16_t                look() const;
    void                    look(uint16_t value);

    uint8_t                 color() const;
    void                    color(uint16_t value);

    uint32_t                defensive_physical() const;
    void                    defensive_physical(uint8_t value);

    uint32_t                defensive_magical() const;
    void                    defensive_magical(uint8_t value);

    void                    base_hp_up(uint32_t value);
    void                    base_mp_up(uint32_t value);

    void                    base_hp(uint32_t value);
    void                    base_mp(uint32_t value);

    uint32_t                base_hp() const;
    uint32_t                base_mp() const;

    uint32_t                id() const;
    fb::game::nation        nation() const;
    bool                    nation(fb::game::nation value);

    fb::game::creature      creature() const;
    bool                    creature(fb::game::creature value);

    uint8_t                 level() const;
    void                    level(uint8_t value);
    bool                    level_up();
    bool                    max_level() const;

    fb::game::sex           sex() const;
    void                    sex(fb::game::sex value);

    fb::game::state         state() const;
    void                    state(fb::game::state value);

    uint8_t                 cls() const;
    void                    cls(uint8_t value);

    uint8_t                 promotion() const;
    void                    promotion(uint8_t value);

    uint8_t                 strength() const;
    void                    strength(uint8_t value);
    void                    strength_up(uint8_t value);

    uint8_t                 intelligence() const;
    void                    intelligence(uint8_t value);
    void                    intelligence_up(uint8_t value);

    uint8_t                 dexteritry() const;
    void                    dexteritry(uint8_t value);
    void                    dexteritry_up(uint8_t value);

    uint32_t                experience() const;
    uint32_t                experience_add(uint32_t value);
    uint32_t                experience_reduce(uint32_t value);
    uint32_t                experience_remained() const;

    uint32_t                money() const;
    void                    money(uint32_t value);
    uint32_t                money_add(uint32_t value);
    uint32_t                money_reduce(uint32_t value);

    uint32_t                random_damage() const;
    void                    random_damage(uint8_t value);

    uint32_t                random_damage(fb::game::life& life, bool& critical) const;

    uint32_t                hit() const;
    void                    hit(uint8_t value);

    uint32_t                regenerative() const;
    void                    regenerative(uint8_t value);

    bool                    option(options key) const;
    void                    option(options key, bool value);
    bool                    option_toggle(options key);

    const std::vector<legend>&  legends() const;
    void                    legends_add(uint8_t look, uint8_t color, const std::string& content);

    const std::string&      title() const;
    void                    title(const std::string& value);

    fb::game::map*          map() const;
    uint16_t                map(fb::game::map* map);
    uint16_t                map(fb::game::map* map, const point16_t position);

    fb::game::group*        group() const;
    bool                    group_enter(fb::game::group* gs);
    bool                    group_leave();

    void                    state_assert(fb::game::state flags) const;
    void                    state_assert(uint8_t flags) const;

public:
    fb::ostream             make_id_stream() const;
    fb::ostream             make_state_stream(fb::game::state_level level) const;
    fb::ostream             make_show_objects_stream() const;
    fb::ostream             make_position_stream() const;
    fb::ostream             make_visual_stream(bool light) const;
    fb::ostream             make_effet_stream(uint8_t effect) const;


    fb::ostream             make_update_item_slot_stream(uint8_t index) const;
    fb::ostream             make_delete_item_slot_stream(fb::game::item::delete_attr types, uint32_t slot, uint16_t count = 0) const;
    fb::ostream             make_update_equipment_stream(fb::game::equipment::slot slot) const;
    fb::ostream             make_equipment_off_stream(fb::game::equipment::slot slot) const;
    fb::ostream             make_internal_info_stream() const;
    fb::ostream             make_external_info_stream() const;
    fb::ostream             make_option_stream() const;
    fb::ostream             make_chat_stream(const std::string& message, bool shout) const;

public:
	IMPLEMENT_NEW_LUA

public:
	static int				builtin_name(lua_State* lua);
	static int				builtin_look(lua_State* lua);
	static int				builtin_color(lua_State* lua);
	static int				builtin_position(lua_State* lua);
	static int				builtin_money(lua_State* lua);
	static int				builtin_exp(lua_State* lua);
	static int				builtin_base_hp(lua_State* lua);
	static int				builtin_base_mp(lua_State* lua);
	static int				builtin_strength(lua_State* lua);
	static int				builtin_dexterity(lua_State* lua);
	static int				builtin_intelligence(lua_State* lua);
};

} }

#endif // !__MMO_H__
