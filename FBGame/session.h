#ifndef __MMO_H__
#define	__MMO_H__

#include <stdint.h>
#include <string>
#include "socket.h"
#include "acceptor.h"
#include <algorithm>
#include "item.h"
#include "object.h"

namespace fb { namespace game {

class map;
class item;
class session;

class trade_system
{
private:
	session*				_owner;
	std::vector<item*>		_list;
	item*					_selected;
	uint32_t				_money;
	bool					_locked;

public:
	trade_system(session& owner);
	~trade_system();

private:
	uint8_t					contains_core(item* item) const;

public:
	item*					selected();
	void					select(item* item);

	uint8_t					add(item* item);
	void					money(uint32_t value);
	uint32_t				money() const;
	std::vector<uint8_t>	restore();
	void					flush(session& session, std::vector<uint8_t>& indices);
	bool					flushable(session& session) const;

	bool					lock() const;
	void					lock(bool value);

public:
	fb::ostream				make_show_stream(bool mine, uint8_t index) const;
	fb::ostream				make_money_stream(bool mine) const;
	fb::ostream				make_dialog_stream() const;
	fb::ostream				make_bundle_stream() const;
	fb::ostream				make_cancel_stream(const std::string& message) const;
	fb::ostream				make_lock_stream() const;
};

class session : public fb_session, public life
{
private:
	uint8_t					_strength, _intelligence, _dexteritry;
	uint8_t					_damage; // 공격수정
	uint8_t					_hit; // 명중수정
	uint8_t					_regenerative; // 재생력

	fb::game::nation		_nation;
	fb::game::creature		_creature;
	fb::game::sex			_sex;
	uint8_t					_level;
	uint8_t					_class, _promotion;
	uint32_t				_money;

	bool					_options[0x0B+1];
	std::vector<legend>		_legends;

	std::string				_title;

	item*					_items[item::MAX_ITEM_SLOT];
	weapon*					_weapon;
	armor*					_armor;
	helmet*					_helmet;
	shield*					_shield;
	ring*					_rings[2];
	auxiliary*				_auxiliaries[2];

	trade_system			_trade_system;

public:
	session(SOCKET socket);
	~session();

public:
	uint32_t				id() const;
	fb::game::nation		nation() const;
	bool					nation(fb::game::nation value);

	fb::game::creature		creature() const;
	bool					creature(fb::game::creature value);

	uint8_t					level() const;
	void					level(uint8_t value);
	bool					level_up();
	bool					max_level() const;

	fb::game::sex			sex() const;
	void					sex(fb::game::sex value);

	uint32_t				base_hp() const;
	void					base_hp(uint32_t value);
	void					base_hp_up(uint32_t value);

	uint32_t				base_mp() const;
	void					base_mp(uint32_t value);
	void					base_mp_up(uint32_t value);

	uint8_t					cls() const;
	void					cls(uint8_t value);

	uint8_t					promotion() const;
	void					promotion(uint8_t value);

	uint8_t					strength() const;
	void					strength(uint8_t value);
	void					strength_up(uint8_t value);

	uint8_t					intelligence() const;
	void					intelligence(uint8_t value);
	void					intelligence_up(uint8_t value);

	uint8_t					dexteritry() const;
	void					dexteritry(uint8_t value);
	void					dexteritry_up(uint8_t value);

	uint32_t				experience() const;
	uint32_t				experience_add(uint32_t value);
	uint32_t				experience_reduce(uint32_t value);
	uint32_t				experience_remained() const;

	uint32_t				money() const;
	void					money(uint32_t value);
	uint32_t				money_add(uint32_t value);
	uint32_t				money_reduce(uint32_t value);

	uint32_t				random_damage() const;
	void					random_damage(uint8_t value);

	uint32_t				random_damage(fb::game::life& life, bool& critical) const;

	uint32_t				hit() const;
	void					hit(uint8_t value);

	uint32_t				regenerative() const;
	void					regenerative(uint8_t value);

	bool					option(options key) const;
	void					option(options key, bool value);
	bool					option_toggle(options key);

	const std::vector<legend>&	legends() const;
	void					legends_add(uint8_t look, uint8_t color, const std::string& content);

	const std::string&		title() const;
	void					title(const std::string& value);

	item*					item(uint8_t index) const;
	bool					item_add(fb::game::item& item);
	uint8_t					item_add(fb::game::item* item);
	bool					item_remove(uint8_t index);
	bool					item_reduce(uint8_t index, uint16_t count);
	bool					item_active(uint8_t index, fb::game::item** activated, uint8_t* updated_index, fb::game::equipment::eq_slots& slot, std::string& message);
	uint8_t					item2index(fb::game::item& item) const;
	bool					equipment_on(uint8_t index, fb::game::equipment::eq_slots& slot, uint8_t* updated_index, std::string& message);
	uint8_t					equipment_off(fb::game::equipment::eq_slots slot, std::string& message);

	fb::game::weapon*		weapon() const;
	fb::game::weapon*		weapon(fb::game::weapon* weapon);

	fb::game::armor*		armor() const;
	fb::game::armor*		armor(fb::game::armor* armor);

	fb::game::shield*		shield() const;
	fb::game::shield*		shield(fb::game::shield* shield);

	fb::game::helmet*		helmet() const;
	fb::game::helmet*		helmet(fb::game::helmet* helmet);

	fb::game::ring*			ring(equipment::EQUIPMENT_POSITION position) const;
	fb::game::ring*			ring(fb::game::ring* ring);

	fb::game::auxiliary*	auxiliary(equipment::EQUIPMENT_POSITION position) const;
	fb::game::auxiliary*	auxiliary(fb::game::auxiliary* auxiliary);

	bool					inventory_free() const;
	uint8_t					inventory_free_size() const;

	fb::game::map*			map() const;
	uint16_t				map(fb::game::map* map);

	trade_system&			trade_system();

public:
	fb::ostream				make_id_stream() const;
	fb::ostream				make_state_stream(fb::game::state_level level) const;
	fb::ostream				make_show_objects_stream() const;
	fb::ostream				make_position_stream() const;
	fb::ostream				make_visual_stream(bool light) const;
	fb::ostream				make_effet_stream(uint8_t effect) const;


	fb::ostream				make_update_item_slot_stream(uint8_t index) const;
	fb::ostream				make_delete_item_slot_stream(fb::game::item::delete_attr types, uint32_t slot, uint16_t count = 0) const;

	fb::ostream				make_update_equipment_stream(fb::game::equipment::eq_slots slot) const;
	fb::ostream				make_equipment_off_stream(fb::game::equipment::eq_slots slot) const;

	fb::ostream				make_internal_info_stream() const;
	fb::ostream				make_external_info_stream() const;

	fb::ostream				make_option_stream() const;
};

} }

#endif // !__MMO_H__
