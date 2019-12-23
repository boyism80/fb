#include "session.h"
#include "map.h"
#include "item.h"
#include "fb_game.h"
using namespace fb::game;

//
// trade system
//
fb::game::trade_system::trade_system(session& owner) : 
	_owner(&owner),
	_selected(NULL),
	_money(0),
	_locked(false),
	_partner(NULL)
{
}

fb::game::trade_system::~trade_system()
{
}

uint8_t fb::game::trade_system::contains_core(fb::game::item* item) const
{
	for(int i = 0; i < this->_list.size(); i++)
	{
		if(this->_list[i]->core() == item->core())
			return i;
	}

	return 0xFF;
}

session* fb::game::trade_system::partner() const
{
	return this->_partner;
}

bool fb::game::trade_system::begin(session* partner)
{
	if(this->_partner != NULL)
		return false;

	this->_partner = partner;
	return true;
}

bool fb::game::trade_system::end()
{
	if(this->_partner == NULL)
		return false;

	this->_partner = NULL;
	return true;
}

bool fb::game::trade_system::trading() const
{
	return this->_partner != NULL;
}

item* fb::game::trade_system::selected()
{
	return this->_selected;
}

void fb::game::trade_system::select(fb::game::item* item)
{
	this->_selected = item;
}

uint8_t fb::game::trade_system::add(fb::game::item* item)
{
	if(item->attr() & fb::game::item::attrs::ITEM_ATTR_BUNDLE)
	{
		uint8_t exists = this->contains_core(item);
		if(exists != 0xFF)
		{
			uint16_t count = this->_list[exists]->count() + item->count();
			this->_list[exists]->count(count);
			return exists;
		}
	}

	this->_list.push_back(item);
	return this->_list.size() - 1;
}

void fb::game::trade_system::money(uint32_t value)
{
	this->_money = value;
}

uint32_t fb::game::trade_system::money() const
{
	return this->_money;
}

std::vector<uint8_t> fb::game::trade_system::restore()
{
	std::vector<uint8_t> indices;
	for(auto it = this->_list.begin(); it != this->_list.end(); it++)
	{
		fb::game::item* item = *it;

		for(int i = 0; i < item::MAX_ITEM_SLOT; i++)
		{
			fb::game::item* own_item = this->_owner->item(i);
			if(own_item == NULL)
				continue;

			if(!(own_item->attr() & item::attrs::ITEM_ATTR_BUNDLE))
				continue;

			if(own_item->core() != item->core())
				continue;

			own_item->count(own_item->count() + item->count());
			indices.push_back(i);

			delete item;
			item = NULL;
			break;
		}

		if(item == NULL)
			continue;

		uint8_t index = this->_owner->item_add(item);
		if(index != 0xFF)
			indices.push_back(index);
	}

	this->_list.clear();

	this->_owner->money_add(this->_money);
	this->_money = 0;

	this->_locked = false;
	return indices;
}

void fb::game::trade_system::flush(session& session, std::vector<uint8_t>& indices)
{
	indices.clear();

	for(auto i = this->_list.begin(); i != this->_list.end(); i++)
	{
		fb::game::item* item = *i;
		indices.push_back(session.item_add(item));

		if(item->empty())
			delete item;
	}
	this->_list.clear();

	session.money_add(this->_money);
	this->_money = 0;

	this->_locked = false;
}

bool fb::game::trade_system::flushable(session& session) const
{
	uint32_t before_money = session.money();
	uint32_t remain = session.money_add(this->_money);
	session.money(before_money);
	
	if(remain != 0)
		return false;

	uint8_t free_size = session.inventory_free_size();
	for(int i = 0; i < item::MAX_ITEM_SLOT; i++)
	{
		fb::game::item* item = session.item(i);
		if(item == NULL)
			continue;

		if(!(item->attr() & item::attrs::ITEM_ATTR_BUNDLE))
			continue;

		uint8_t contains_core = this->contains_core(item);
		if(contains_core == 0xFF)
			continue;

		if(item->free_space() < this->_list[contains_core]->count())
			return false;

		free_size++;
	}

	if(free_size < this->_list.size())
		return false;

	return true;
}

bool fb::game::trade_system::lock() const
{
	return this->_locked;
}

void fb::game::trade_system::lock(bool value)
{
	this->_locked = value;
}

fb::ostream fb::game::trade_system::make_show_stream(bool mine, uint8_t index) const
{
	try
	{
		fb::ostream				ostream;
		fb::game::item*			item = this->_list[index];
		const std::string		name = item->name_trade();

		ostream.write_u8(0x42)
			.write_u8(0x02)
			.write_u8(mine ? 0x00 : 0x01)
			.write_u8(index) // trade slot index
			.write_u16(item->look())
			.write_u8(item->color())
			.write_u8(name.length())
			.write(name.c_str(), name.length())
			.write_u8(0x00);

		return ostream;
	}
	catch(std::exception& e)
	{
		return fb::ostream();
	}
}

fb::ostream fb::game::trade_system::make_money_stream(bool mine) const
{
	fb::ostream				ostream;

	ostream.write_u8(0x42)
		.write_u8(0x03)
		.write_u8(mine ? 0x00 : 0x01)
		.write_u32(this->_money)
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::trade_system::make_dialog_stream() const
{
	fb::ostream             ostream;
	const std::string*		class_name = acceptor::instance()->class2name(this->_owner->cls(), this->_owner->promotion());
	if(class_name == NULL)
		return ostream;

	std::stringstream sstream;
	sstream << this->_owner->name() << '(' << class_name->c_str() << ')';
	std::string your_name = sstream.str();

	ostream.write_u8(0x42)
		.write_u8(0x00)
		.write_u32(this->_owner->id())
		.write_u8(your_name.length())
		.write(your_name.c_str(), your_name.length())
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::trade_system::make_bundle_stream() const
{
	fb::ostream				ostream;

	ostream.write_u8(0x42)
		.write_u8(0x01)
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::trade_system::make_close_stream(const std::string& message) const
{
	fb::ostream             ostream;

	ostream.write_u8(0x42)
		.write_u8(0x04)
		.write_u16(message.length())
		.write(message.c_str(), message.length())
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::trade_system::make_lock_stream() const
{
	fb::ostream				ostream;

	ostream.write_u8(0x42)
		.write_u8(0x05)
		.write_u8(0x00);

	return ostream;
}





//
// class session
//
session::session(SOCKET socket) : 
	fb_session(socket),
	life(socket, "", 0, 0),
	_nation(fb::game::nation::GOGURYEO),
	_creature(fb::game::creature::DRAGON),
	_state(state::NORMAL),
	_level(1),
	_class(0),
	_promotion(0),
	_money(0),
	_weapon(NULL), _armor(NULL), _helmet(NULL), _shield(NULL),
	_trade_system(*this)
{
	memset(this->_items, NULL, sizeof(this->_items));
	memset(this->_rings, NULL, sizeof(this->_rings));
	memset(this->_auxiliaries, NULL, sizeof(this->_auxiliaries));
	memset(this->_options, 0, sizeof(this->_options));
}

session::~session()
{
	for(int i = 0; i < item::MAX_ITEM_SLOT; i++)
	{
		if(this->_items[i])
			delete this->_items[i];
	}

	if(this->_weapon)
		delete this->_weapon;

	if(this->_armor)
		delete this->_armor;

	if(this->_helmet)
		delete this->_helmet;

	if(this->_shield)
		delete this->_shield;

	if(this->_rings[0])
		delete this->_rings[0];

	if(this->_rings[1])
		delete this->_rings[1];

	if(this->_auxiliaries[0])
		delete this->_auxiliaries[0];

	if(this->_auxiliaries[1])
		delete this->_auxiliaries[1];
}

//uint16_t fb::game::session::look() const
//{
//	return this->_look;
//}

//void fb::game::session::look(uint16_t value)
//{
//	this->_core->look(value);
//}

//uint8_t fb::game::session::color() const
//{
//	return this->_core->color();
//}

//void fb::game::session::color(uint16_t value)
//{
//	this->_core->color(value);
//}

uint32_t fb::game::session::id() const
{
	return this->_id;
}

fb::game::nation fb::game::session::nation() const
{
	return this->_nation;
}

bool fb::game::session::nation(fb::game::nation value)
{
	if(value != fb::game::nation::GOGURYEO &&
		value != fb::game::nation::BUYEO)
		return false;

	this->_nation = value;
	return true;
}

fb::game::creature fb::game::session::creature() const
{
	return this->_creature;
}

bool fb::game::session::creature(fb::game::creature value)
{
	if(value != fb::game::creature::DRAGON &&
		value != fb::game::creature::PHOENIX &&
		value != fb::game::creature::TIGER &&
		value != fb::game::creature::TURTLE)
		return false;

	this->_creature = value;
	return true;
}

uint8_t fb::game::session::level() const
{
	return this->_level;
}

void fb::game::session::level(uint8_t value)
{
	this->_level = value;
}

bool fb::game::session::level_up()
{
	if(this->max_level())
		return false;

	this->_level++;
	return true;
}

bool fb::game::session::max_level() const
{
	return this->_level >= 99;
}

fb::game::sex fb::game::session::sex() const
{
	return this->_sex;
}

void fb::game::session::sex(fb::game::sex value)
{
	this->_sex = value;
}

fb::game::state fb::game::session::state() const
{
	return this->_state;
}

void fb::game::session::state(fb::game::state value)
{
	this->_state = value;
}

uint32_t fb::game::session::base_hp() const
{
	return fb::game::life::base_hp();
}

void fb::game::session::base_hp(uint32_t value)
{
	fb::game::life::base_hp(value);
}

void fb::game::session::base_hp_up(uint32_t value)
{
	uint32_t capacity = 0xFFFFFFFF - this->_base_hp;
	this->_base_hp += std::min(capacity, value);
}

uint32_t fb::game::session::base_mp() const
{
	return this->_base_mp;
}

void fb::game::session::base_mp(uint32_t value)
{
	this->_base_mp = value;
}

void fb::game::session::base_mp_up(uint32_t value)
{
	uint32_t capacity = 0xFFFFFFFF - this->_base_mp;
	this->_base_mp += std::min(capacity, value);
}

uint8_t fb::game::session::cls() const
{
	return this->_class;
}

void fb::game::session::cls(uint8_t value)
{
	this->_class = value;
}

uint8_t fb::game::session::promotion() const
{
	return uint8_t();
}

void fb::game::session::promotion(uint8_t value)
{
	this->_promotion = value;
}

uint8_t fb::game::session::strength() const
{
	return this->_strength;
}

void fb::game::session::strength(uint8_t value)
{
	this->_strength = value;
}

void fb::game::session::strength_up(uint8_t value)
{
	this->_strength + value;
}

uint8_t fb::game::session::intelligence() const
{
	return this->_intelligence;
}

void fb::game::session::intelligence(uint8_t value)
{
	this->_intelligence = value;
}

void fb::game::session::intelligence_up(uint8_t value)
{
	this->_intelligence += value;
}

uint8_t fb::game::session::dexteritry() const
{
	return this->_dexteritry;
}

void fb::game::session::dexteritry(uint8_t value)
{
	this->_dexteritry = value;
}

void fb::game::session::dexteritry_up(uint8_t value)
{
	this->_dexteritry += value;
}

uint32_t fb::game::session::experience() const
{
	return this->_experience;
}

uint32_t fb::game::session::experience_add(uint32_t value)
{
	uint32_t capacity = 0xFFFFFFFF - this->_experience;
	
	if(value > capacity)
	{
		this->_experience += capacity;
		return value - capacity;
	}
	else
	{
		this->_experience += value;
		return 0;
	}
}

uint32_t fb::game::session::experience_reduce(uint32_t value)
{
	if(this->_experience < value)
	{
		uint32_t lack = value - this->_experience;
		this->_experience = 0;
		return lack;
	}
	else
	{
		this->_experience -= value;
		return 0;
	}
}

uint32_t fb::game::session::experience_remained() const
{
	if(this->max_level())
		return 0;

	if(this->_class == 0 && this->_level >= 5)
		return 0;

	return fb::game::acceptor::instance()->required_exp(this->_class, this->_level+1) - this->experience();
}

uint32_t fb::game::session::money() const
{
	return this->_money;
}

void fb::game::session::money(uint32_t value)
{
	this->_money = value;
}

uint32_t fb::game::session::money_add(uint32_t value) // 먹고 남은 값 리턴
{
	uint32_t capacity = 0xFFFFFFFF - this->_money;
	if(value > capacity)
	{
		this->_money += capacity;
		return value - capacity;
	}
	else
	{
		this->_money += value;
		return 0;
	}
}

uint32_t fb::game::session::money_reduce(uint32_t value)
{
	if(this->_money < value)
	{
		uint32_t lack = value - this->_money;
		this->_money = 0;
		return lack;
	}
	else
	{
		this->_money -= value;
		return 0;
	}
}

uint32_t fb::game::session::random_damage() const
{
	return this->_damage;
}

void fb::game::session::random_damage(uint8_t value)
{
	this->_damage = value;
}

uint32_t fb::game::session::random_damage(fb::game::life& life, bool& critical) const
{
	uint32_t random_damage = 0;

	if(this->_weapon == NULL) // no weapon
	{
		random_damage = 1 + std::rand() % 5;
		critical = false;
	}
	else if(std::rand() % 100 < 20) // critical
	{
		fb::game::range32_t range = this->_weapon->damage_large();
		random_damage = std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
		critical = true;
	}
	else // normal
	{
		fb::game::range32_t range = this->_weapon->damage_small();
		random_damage = std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
		critical = false;
	}

	return __super::random_damage(random_damage, life);
}

uint32_t fb::game::session::hit() const
{
	return this->_hit;
}

void fb::game::session::hit(uint8_t value)
{
	this->_hit = value;
}

uint32_t fb::game::session::regenerative() const
{
	return this->_regenerative;
}

void fb::game::session::regenerative(uint8_t value)
{
	this->_regenerative = value;
}

bool fb::game::session::option(options key) const
{
	if(key > 0x1B)
		return false;

	return this->_options[key];
}

void fb::game::session::option(options key, bool value)
{
	if(key > 0x1B)
		return;

	this->_options[key] = value;
}

bool fb::game::session::option_toggle(options key)
{
	if(key > 0x1B)
		return false;

	this->_options[key] = !this->_options[key];
	return this->_options[key];
}

const std::vector<legend>& fb::game::session::legends() const
{
	return this->_legends;
}

void fb::game::session::legends_add(uint8_t look, uint8_t color, const std::string& content)
{
	this->_legends.push_back(legend(look, color, content));
}

const std::string& fb::game::session::title() const
{
	return this->_title;
}

void fb::game::session::title(const std::string& value)
{
	this->_title = value;
}

item* fb::game::session::item(uint8_t index) const
{
	if(index > fb::game::item::MAX_ITEM_SLOT - 1)
		return NULL;

	return this->_items[index];
}

bool fb::game::session::item_add(fb::game::item& item)
{
	for(int i = 0; i < fb::game::item::MAX_ITEM_SLOT; i++)
	{
		if(this->_items[i] != NULL)
			continue;

		this->_items[i] = &item;
		return true;
	}

	return false;
}

uint8_t fb::game::session::item_add(fb::game::item* item)
{
	if(item == NULL)
		return -1;


	// 번들 형식의 아이템인 경우
	if(item->attr() & fb::game::item::attrs::ITEM_ATTR_BUNDLE)
	{
		for(int i = 0; i < fb::game::item::MAX_ITEM_SLOT; i++)
		{
			if(this->_items[i] == NULL)
				continue;

			if(item->core() != this->_items[i]->core())
				continue;


			// 아이템을 합치고 남은 갯수로 설정한다.
			uint16_t remain = this->_items[i]->fill(item->count());
			item->count(remain);

			return i;
		}
	}

	// 그 이외의 아이템인 경우
	for(int i = 0; i < fb::game::item::MAX_ITEM_SLOT; i++)
	{
		if(this->_items[i] != NULL)
			continue;

		this->_items[i] = item;
		return i;
	}

	return -1;
}

bool fb::game::session::item_remove(uint8_t index)
{
	fb::game::item* item = this->item(index);
	if(item == NULL)
		return false;

	this->_items[index] = NULL;
	return true;
}

bool fb::game::session::item_reduce(uint8_t index, uint16_t count)
{
	fb::game::item* item = this->item(index);
	if(item == NULL)
		return false;

	this->_items[index]->reduce(count);
	return true;
}

bool fb::game::session::item_active(uint8_t index, fb::game::item** activated, uint8_t* updated_index, fb::game::equipment::eq_slots& slot, std::string& message)
{
	slot = fb::game::equipment::eq_slots::UNKNOWN_SLOT;
	// TODO
	// 말에 탄 상태, 귀신상태 체크
	
	
	fb::game::item* item = this->_items[index];
	if(item == NULL)
		return false;

	fb::game::item::attrs attr = item->attr();
	if((attr & fb::game::item::attrs::ITEM_ATTR_EQUIPMENT) && this->equipment_on(index, slot, updated_index, message) == false)
		return false;


	item->handle_acive(*this);
	if(item->empty())
		this->_items[index] = NULL;

	if(activated != NULL)
		*activated = item;

	return item;
}

uint8_t fb::game::session::item2index(fb::game::item& item) const
{
	for(int i = 0; i < item::MAX_ITEM_SLOT; i++)
	{
		if(this->_items[i] == NULL)
			continue;

		if(this->_items[i]->core() == item.core())
			return i;
	}

	return 0xFF;
}

bool fb::game::session::equipment_on(uint8_t index, fb::game::equipment::eq_slots& slot, uint8_t* updated_index, std::string& message)
{
	if(updated_index != NULL)
		*updated_index = 0xFF;

	fb::game::item* item = this->item(index);
	if(item == NULL)
		return false;

	fb::game::item* before = NULL;
	fb::game::item::attrs attr = fb::game::item::attrs(item->attr() & ~fb::game::item::attrs::ITEM_ATTR_EQUIPMENT);
	switch(attr)
	{
	case fb::game::item::attrs::ITEM_ATTR_WEAPON:
		before = this->weapon(static_cast<fb::game::weapon*>(item));
		slot = fb::game::equipment::eq_slots::WEAPON_SLOT;
		break;

	case fb::game::item::attrs::ITEM_ATTR_ARMOR:
		before = this->armor(static_cast<fb::game::armor*>(item));
		slot = fb::game::equipment::eq_slots::ARMOR_SLOT;
		break;

	case fb::game::item::attrs::ITEM_ATTR_SHIELD:
		before = this->shield(static_cast<fb::game::shield*>(item));
		slot = fb::game::equipment::eq_slots::SHIELD_SLOT;
		break;

	case fb::game::item::attrs::ITEM_ATTR_HELMET:
		before = this->helmet(static_cast<fb::game::helmet*>(item));
		slot = fb::game::equipment::eq_slots::HELMET_SLOT;
		break;

	case fb::game::item::attrs::ITEM_ATTR_RING:
		if(this->_rings[0] == NULL)
		{
			slot = fb::game::equipment::eq_slots::LEFT_HAND_SLOT;
		}
		else
		{
			slot = fb::game::equipment::eq_slots::RIGHT_HAND_SLOT;
		}
		
		before = this->ring(static_cast<fb::game::ring*>(item));
		break;


	case fb::game::item::attrs::ITEM_ATTR_AUXILIARY:
		if(this->_auxiliaries[0] == NULL)
		{
			slot = fb::game::equipment::eq_slots::LEFT_AUX_SLOT;
		}
		else
		{
			slot = fb::game::equipment::eq_slots::RIGHT_AUX_SLOT;
		}

		before = this->auxiliary(static_cast<fb::game::auxiliary*>(item));
		break;
	
	default:
		message = "입을 수 없는 물건입니다.";
		return false;
	}


	this->_items[index] = NULL;
	uint8_t updated = this->item_add(before);
	if(updated_index != NULL)
		*updated_index = updated;
	
	return true;
}

uint8_t fb::game::session::equipment_off(fb::game::equipment::eq_slots slot, std::string& message)
{
	// TODO
	// 말, 귀신 체크

	if(this->inventory_free() == false)
	{
		message = "소지품이 꽉 찼습니다.";
		return -1;
	}

	fb::game::item* item;
	switch(slot)
	{
	case fb::game::equipment::eq_slots::WEAPON_SLOT:
		item = this->_weapon;
		if(this->_weapon != NULL)
			this->_weapon = NULL;

		break;

	case fb::game::equipment::eq_slots::ARMOR_SLOT:
		item = this->_armor;
		if(this->_armor != NULL)
			this->_armor = NULL;
		break;

	case fb::game::equipment::eq_slots::SHIELD_SLOT:
		item = this->_shield;
		if(this->_shield != NULL)
			this->_shield = NULL;
		break;

	case fb::game::equipment::eq_slots::HELMET_SLOT:
		item = this->_helmet;
		if(this->_helmet != NULL)
			this->_helmet = NULL;
		break;

	case fb::game::equipment::eq_slots::LEFT_HAND_SLOT:
		item = this->_rings[0];
		if(this->_rings[0] != NULL)
			this->_rings[0] = NULL;
		break;

	case fb::game::equipment::eq_slots::RIGHT_HAND_SLOT:
		item = this->_rings[1];
		if(this->_rings[1] != NULL)
			this->_rings[1] = NULL;
		break;

	case fb::game::equipment::eq_slots::LEFT_AUX_SLOT:
		item = this->_auxiliaries[0];
		if(this->_auxiliaries[0] != NULL)
			this->_auxiliaries[0] = NULL;
		break;

	case fb::game::equipment::eq_slots::RIGHT_AUX_SLOT:
		item = this->_auxiliaries[1];
		if(this->_auxiliaries[1] != NULL)
			this->_auxiliaries[1] = NULL;
		break;
	}

	return this->item_add(item);
}

weapon* fb::game::session::weapon() const
{
	return this->_weapon;
}

fb::game::weapon* fb::game::session::weapon(fb::game::weapon* weapon)
{
	fb::game::weapon* before = this->_weapon;

	this->_weapon = weapon;
	return before;
}

armor* fb::game::session::armor() const
{
	return this->_armor;
}

fb::game::armor* fb::game::session::armor(fb::game::armor* armor)
{
	fb::game::armor* before = this->_armor;
	
	this->_armor = armor;
	return before;
}

shield* fb::game::session::shield() const
{
	return this->_shield;
}

fb::game::shield* fb::game::session::shield(fb::game::shield* shield)
{
	fb::game::shield* before = this->_shield;

	this->_shield = shield;
	return before;
}

helmet* fb::game::session::helmet() const
{
	return this->_helmet;
}

fb::game::helmet* fb::game::session::helmet(fb::game::helmet* helmet)
{
	fb::game::helmet* before = this->_helmet;

	this->_helmet = helmet;
	return before;
}

ring* fb::game::session::ring(fb::game::equipment::EQUIPMENT_POSITION position) const
{
	return this->_rings[position];
}

fb::game::ring* fb::game::session::ring(fb::game::ring* ring)
{
	fb::game::ring* before = NULL;

	if(this->_rings[0] == NULL)
	{
		before = this->_rings[0];
		this->_rings[0] = ring;
	}
	else
	{
		before = this->_rings[1];
		this->_rings[1] = ring;
	}

	return before;
}


auxiliary* fb::game::session::auxiliary(fb::game::equipment::EQUIPMENT_POSITION position) const
{
	return this->_auxiliaries[position];
}

fb::game::auxiliary* fb::game::session::auxiliary(fb::game::auxiliary* auxiliary)
{
	fb::game::auxiliary* before = NULL;

	if(this->_auxiliaries[0] == NULL)
	{
		before = this->_auxiliaries[0];
		this->_auxiliaries[0] = auxiliary;
	}
	else
	{
		before = this->_auxiliaries[1];
		this->_auxiliaries[1] = auxiliary;
	}

	return before;
}

bool fb::game::session::inventory_free() const
{
	for(int i = 0; i < item::MAX_ITEM_SLOT; i++)
	{
		if(this->_items[i] == NULL)
			return true;
	}

	return false;
}

uint8_t fb::game::session::inventory_free_size() const
{
	uint8_t count = 0;
	for(int i = 0; i < item::MAX_ITEM_SLOT; i++)
	{
		if(this->_items[i] == NULL)
			count++;
	}

	return count;
}

uint16_t fb::game::session::map(fb::game::map* map)
{
	if(this->_map != NULL)
		this->_map->session_delete(this);

	this->_map = map;
	this->_map->session_add(this);
	return this->id();
}

trade_system& fb::game::session::trade_system()
{
	return this->_trade_system;
}

fb::game::group_system* fb::game::session::group_system() const
{
	return this->_group;
}

bool fb::game::session::group_enter(fb::game::group_system* gs)
{
	if(this->_group != NULL)
		return false;

	this->_group = gs;
	return true;
}

bool fb::game::session::group_leave()
{
	if(this->_group == NULL)
		return false;

	this->_group = NULL;
	return true;
}

bool fb::game::session::state_assert(std::string& message, fb::game::state flags) const
{
	if(flags & fb::game::state::GHOST && this->_state == fb::game::GHOST)
	{
		message = "귀신은 할 수 없습니다.";
		return false;
	}

	if(flags & fb::game::state::RIDING && this->_state == fb::game::RIDING)
	{
		message = "말을 타고는 할 수 없습니다.";
		return false;
	}

	if(flags & fb::game::state::DISGUISE && this->_state == fb::game::DISGUISE)
	{
		message = "변신 중에는 할 수 없습니다.";
		return false;
	}

	return true;
}

fb::game::map* fb::game::session::map() const
{
	return __super::map();
}

fb::ostream fb::game::session::make_id_stream() const
{
	fb::ostream             ostream;
	ostream.write_u8(0x05)
		.write_u32(this->id())
		.write_u32(this->direction()) // side
		.write_u32(0x00); // class

	return ostream;
}

fb::ostream fb::game::session::make_state_stream(fb::game::state_level level) const
{
	fb::ostream             ostream;

	ostream.write_u8(0x08) // cmd
		.write_u8(level);

	if(level & state_level::BASED)
	{
		ostream.write_u8(this->nation()) // nation
			.write_u8(this->creature()) // creature
			.write_u8(0x00) // Unknown (clan?)
			.write_u8(this->level()) // level
			.write_u32(this->base_hp()) // base hp
			.write_u32(this->base_mp()) // base mp
			.write_u8(this->strength())
			.write_u8(this->intelligence())
			.write_u8(0x03)
			.write_u8(0x03)
			.write_u8(this->dexteritry())
			.write_u8(0x03)
			.write_u32(0x00)
			.write_u8(0x00);
	}

	if(level & state_level::HP_MP)
	{
		ostream.write_u32(this->base_hp())  // current hp
			.write_u32(this->base_mp()); // current mp
	}

	if(level & state_level::EXP_MONEY)
	{
		ostream.write_u32(this->experience())  // exp
			.write_u32(this->money()); // money
	}

	if(level & state_level::CONDITION)
	{
		ostream.write_u8(this->condition_contains(fb::game::condition::MOVE))  // condition::move
			.write_u8(this->condition_contains(fb::game::condition::SIGHT))  // condition::sight
			.write_u8(this->condition_contains(fb::game::condition::HEAR))  // condition::hear?
			.write_u8(this->condition_contains(fb::game::condition::ORAL))  // condition:oral
			.write_u8(this->condition_contains(fb::game::condition::MAP)); // condition:map?
	}

	ostream.write_u8(0x00)  // mail count
		.write_u8(true)  // fast move
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::session::make_show_objects_stream() const
{
	fb::game::map*          map = this->map();
	if(map == NULL)
		return fb::ostream();

	auto                    objects = map->objects();
	std::vector<object*>    visibles;
	for(auto i = objects.begin(); i != objects.end(); i++)
	{
		fb::game::object*   object = *i;
		if(this->sight(*object) == false)
			continue;

		visibles.push_back(object);
	}

	return fb::game::object::make_show_stream(visibles);
}

fb::ostream fb::game::session::make_position_stream() const
{
	fb::ostream             ostream;
	ostream.write_u8(0x04)
		.write_u16(this->_position.x)  // 실제 x 좌표
		.write_u16(this->_position.y); // 실제 y 좌표

								 // 스크린에서의 x 좌표
	fb::game::map*          map = this->map();
	if(map->width() < map::MAX_SCREEN_WIDTH)
		ostream.write_u16(this->_position.x + map::HALF_SCREEN_WIDTH - (map->width() / 2));
	else if(this->_position.x < map::HALF_SCREEN_WIDTH)
		ostream.write_u16(this->_position.x);
	else if(this->_position.x >= map->width() - map::HALF_SCREEN_WIDTH)
		ostream.write_u16(this->_position.x + map::MAX_SCREEN_WIDTH - map->width());
	else
		ostream.write_u16(map::HALF_SCREEN_WIDTH);

	// 스크린에서의 y 좌표
	if(map->height() < map::MAX_SCREEN_HEIGHT)
		ostream.write_u16(this->_position.y + map::HALF_SCREEN_HEIGHT - (map->height() / 2));
	else if(this->_position.y < map::HALF_SCREEN_HEIGHT)
		ostream.write_u16(this->_position.y);
	else if(this->_position.y >= (map->height() - map::HALF_SCREEN_HEIGHT))
		ostream.write_u16(this->_position.y + map::MAX_SCREEN_HEIGHT - map->height());
	else
		ostream.write_u16(map::HALF_SCREEN_HEIGHT);

	ostream.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::session::make_visual_stream(bool light) const
{
	fb::ostream             ostream;

	ostream.write_u8(light ? 0x1D : 0x33); // cmd
	if(light == false)
	{
		ostream.write_u16(this->_position.x) // x
			.write_u16(this->_position.y) // y
			.write_u8(this->_direction); // side
	}

	ostream.write_u32(this->id())
		.write_u8(this->_state == fb::game::state::DISGUISE) // 변신유무
		.write_u8(this->_sex) // sex
		.write_u8(this->_state) // state
		.write_u16(this->look()) // face
		.write_u8(this->color()); // hair color

	if(this->_armor != NULL)
	{
		ostream.write_u8(this->_armor->dress())
			.write_u8(this->_armor->color());
	}
	else
	{
		ostream.write_u8((uint8_t)this->_sex) // sex
			.write_u8(0x00);
	}

	if(this->_weapon != NULL)
	{
		ostream.write_u16(this->_weapon->dress())
			.write_u8(this->_weapon->color());
	}
	else
	{
		ostream.write_u16(0xFFFF)
			.write_u8(0x00);
	}

	if(this->_shield != NULL)
	{
		ostream.write_u8(this->_shield->dress())
			.write_u8(this->_shield->color());
	}
	else
	{
		ostream.write_u8(0xFF) // about shield
			.write_u8(0x00);
	}

	const std::string& name = this->name();
	ostream.write_u8(0x04) // head mark
		.write_u8((uint8_t)name.length()) // name length
		.write(name.c_str(), name.length() + 1); // name

	return ostream;
}

fb::ostream fb::game::session::make_effet_stream(uint8_t effect) const
{
	fb::ostream				ostream;

	ostream.write_u8(0x29)
		.write_u32(this->id())
		.write_u8(effect)
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::session::make_update_item_slot_stream(uint8_t index) const
{
	fb::ostream             ostream;
	fb::game::item*         item = this->item(index);
	if(item == NULL)
		return ostream;

	const std::string name = item->name_styled();

	ostream.write_u8(0x0F)
		.write_u8(index + 1)
		.write_u16(item->look())
		.write_u8(item->color())
		.write_u8(name.length())
		.write(name.c_str(), name.length())
		.write_u32(item->count())
		.write_u8(0x00)
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::session::make_delete_item_slot_stream(fb::game::item::delete_attr types, uint32_t slot, uint16_t count) const
{
	fb::ostream             ostream;
	if(slot + 1 > item::MAX_ITEM_SLOT)
		return ostream;

	ostream.write_u8(0x10)
		.write_u8(slot + 1)
		.write_u8(types)
		.write_u16(count);

	return ostream;
}

fb::ostream fb::game::session::make_update_equipment_stream(fb::game::equipment::eq_slots slot) const
{
	fb::ostream             ostream;
	fb::game::item*         item;

	switch(slot)
	{
	case fb::game::equipment::eq_slots::WEAPON_SLOT:
		item = this->weapon();
		break;

	case fb::game::equipment::eq_slots::ARMOR_SLOT:
		item = this->armor();
		break;

	case fb::game::equipment::eq_slots::SHIELD_SLOT:
		item = this->shield();
		break;

	case fb::game::equipment::eq_slots::HELMET_SLOT:
		item = this->helmet();
		break;

	case fb::game::equipment::eq_slots::LEFT_HAND_SLOT:
		item = this->ring(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT);
		break;

	case fb::game::equipment::eq_slots::RIGHT_HAND_SLOT:
		item = this->ring(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT);
		break;

	case fb::game::equipment::eq_slots::LEFT_AUX_SLOT:
		item = this->auxiliary(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT);
		break;

	case fb::game::equipment::eq_slots::RIGHT_AUX_SLOT:
		item = this->auxiliary(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT);
		break;

	default:
		return false;
	}

	if(item == NULL)
		return ostream;

	const std::string& name = item->name();
	ostream.write_u8(0x37)
		.write_u16(item->look())
		.write_u8(item->color())
		.write_u8(name.length())
		.write(name.c_str(), name.length());

	return ostream;
}

fb::ostream fb::game::session::make_equipment_off_stream(fb::game::equipment::eq_slots slot) const
{
	fb::ostream             ostream;
	ostream.write_u8(0x38)
		.write_u8(slot)
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::session::make_internal_info_stream() const
{
	//
	// 키보드 S를 눌렀을 때 표현될 자신의 캐릭터 정보
	//

	fb::ostream             ostream;

	ostream.write_u8(0x39)
		.write_u8(this->_defensive.physical)
		.write_u8(this->random_damage())
		.write_u8(this->hit())
		.write_u8(0x00) // 클랜정보 없음
		.write_u8(0x00); // 클랜 타이틀 없음

	ostream.write_u8(this->_title.size());
	if(this->_title.empty() == false)
		ostream.write(this->_title.c_str(), this->_title.size());

	std::string             group_message = "그룹 없음.";
	ostream.write_u8(group_message.size())
		.write(group_message.c_str(), group_message.size());

	ostream.write_u8(this->option(options::GROUP)); // 그룹허가


	// 남은 경험치
	uint32_t remained_exp = this->experience_remained();
	ostream.write_u32(remained_exp);

	const std::string*             class_name = fb::game::acceptor::instance()->class2name(this->_class, this->_promotion);
	if(class_name == NULL)
		return fb::ostream();

	ostream.write_u8((*class_name).size())
		.write((*class_name).c_str(), (*class_name).size());

	fb::game::equipment* equipments[] = {this->helmet(), this->ring(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT), this->ring(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT), this->auxiliary(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT), this->auxiliary(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT)};
	for(int i = 0, size = sizeof(equipments) / sizeof(fb::game::equipment*); i < size; i++)
	{
		if(equipments[i] == NULL)
		{
			ostream.write_u16(0xFFFF)
				.write_u8(0x00);
		}
		else
		{
			ostream.write_u16(equipments[i]->look())
				.write_u8(equipments[i]->color());
		}
	}

	ostream.write_u8(0x00) // fixed;
		.write_u8(this->_options[options::TRADE]) // trade
		.write_u8(this->_options[options::PK]); // pk

	ostream.write_u8(this->_legends.size());
	for(auto i = this->_legends.cbegin(); i != this->_legends.cend(); i++)
	{
		ostream.write_u8((*i).look)
			.write_u8((*i).color)
			.write_u8((*i).content.size())
			.write((*i).content.c_str(), (*i).content.size());
	}
	ostream.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::session::make_external_info_stream() const
{
	//
	// 다른 캐릭터를 클릭했을 때 표현될 정보
	//

	fb::ostream				ostream;

	ostream.write_u8(0x34);

	// 유저 타이틀
	ostream.write_u8(this->_title.size());
	if(this->_title.empty() == false)
		ostream.write(this->_title.c_str(), this->_title.size());

	// 클랜 타이틀
	ostream.write_u8(0x00);

	// 클랜 이름
	ostream.write_u8(0x00);

	// 클래스 이름
	const std::string* class_name = fb::game::acceptor::instance()->class2name(this->_class, this->_promotion);
	if(class_name == NULL)
		return fb::ostream();

	ostream.write_u8((*class_name).size())
		.write((*class_name).c_str(), (*class_name).size());

	// 캐릭터 이름
	const std::string& name = this->name();
	ostream.write_u8(name.size())
		.write(name.c_str(), name.size());

	// 캐릭터 상태
	bool transform = false;
	if(transform) // 변신상태
	{
		ostream.write_u8(0x01);
	}
	else // 일반상태
	{
		ostream.write_u8(0x00)
			.write_u8(this->_sex)
			.write_u8(0x00) // state
			.write_u16(this->look())
			.write_u8(this->color());
	}


	// 장비정보
	std::stringstream sstream;
	fb::game::armor* armor = this->armor(); // 갑옷
	ostream.write_u8(armor != NULL ? armor->dress() : 0xFF)
		.write_u8(armor != NULL ? armor->color() : 0x00);

	fb::game::weapon* weapon = this->weapon(); // 무기
	ostream.write_u16(weapon != NULL ? weapon->dress() : 0xFFFF)
		.write_u8(weapon != NULL ? weapon->color() : 0x00);

	fb::game::shield* shield = this->shield(); // 방패
	ostream.write_u8(shield != NULL ? shield->dress() : 0xFF)
		.write_u8(shield != NULL ? shield->color() : 0x00);

	fb::game::helmet* helmet = this->helmet(); // 투구
	ostream.write_u16(helmet != NULL ? helmet->look() : 0xFFFF)
		.write_u8(helmet != NULL ? helmet->color() : 0x00);

	fb::game::ring* ring_l = this->ring(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT); // 왼손
	ostream.write_u16(ring_l != NULL ? ring_l->look() : 0xFFFF)
		.write_u8(ring_l != NULL ? ring_l->color() : 0x00);

	fb::game::ring* ring_r = this->ring(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT); // 오른손
	ostream.write_u16(ring_r != NULL ? ring_r->look() : 0xFFFF)
		.write_u8(ring_r != NULL ? ring_r->color() : 0x00);

	fb::game::auxiliary* aux_l = this->auxiliary(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT); // 보조1
	ostream.write_u16(aux_l != NULL ? aux_l->look() : 0xFFFF)
		.write_u8(aux_l != NULL ? aux_l->color() : 0x00);

	fb::game::auxiliary* aux_r = this->auxiliary(fb::game::equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT); // 보조2
	ostream.write_u16(aux_r != NULL ? aux_r->look() : 0xFFFF)
		.write_u8(aux_r != NULL ? aux_r->color() : 0x00);


	// 장비정보 텍스트
	sstream << " w:무기  :" << (weapon != NULL ? weapon->name() : "없음") << std::endl;
	sstream << " a:갑옷  :" << (armor  != NULL ? armor->name()  : "없음") << std::endl;
	sstream << " s:방패  :" << (shield != NULL ? shield->name() : "없음") << std::endl;
	sstream << " h:머리  :" << (helmet != NULL ? helmet->name() : "없음") << std::endl;
	sstream << " l:왼손  :" << (ring_l != NULL ? ring_l->name() : "없음") << std::endl;
	sstream << " r:오른손:" << (ring_r != NULL ? ring_r->name() : "없음") << std::endl;
	sstream << " [:보조1 :" << (aux_l  != NULL ? aux_l->name()  : "없음") << std::endl;
	sstream << " ]:보조2 :" << (aux_r  != NULL ? aux_r->name()  : "없음") << std::endl;
	ostream.write_u8(sstream.str().size())
		.write(sstream.str().c_str(), sstream.str().size());


	ostream.write_u32(this->id())
		.write_u8(this->_options[fb::game::options::GROUP])
		.write_u8(this->_options[fb::game::options::TRADE])
		.write_u32(0x00000000); // unknown

								// 업적
	ostream.write_u8(this->_legends.size());
	for(auto i = this->_legends.cbegin(); i != this->_legends.cend(); i++)
	{
		ostream.write_u8((*i).look)
			.write_u8((*i).color)
			.write_u8((*i).content.size())
			.write((*i).content.c_str(), (*i).content.size());
	}
	ostream.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::session::make_option_stream() const
{
	fb::ostream             ostream;
	ostream.write_u8(0x23)
		.write_u8(this->_options[options::WEATHER_EFFECT]) // weather
		.write_u8(this->_options[options::MAGIC_EFFECT]) // magic effect
		.write_u8(this->_options[options::ROAR_WORLDS]) // listen news
		.write_u8(this->_options[options::FAST_MOVE]) // fast move
		.write_u8(this->_options[options::EFFECT_SOUND]) // effect sound
		.write_u8(0x00);

	return ostream;
}

fb::game::group_system::group_system(session& leader) : 
	_leader(&leader)
{
	this->add(leader);
}

fb::game::group_system::~group_system()
{
	for(auto i = this->_members.begin(); i != this->_members.end(); i++)
		(*i)->group_leave();
}

bool fb::game::group_system::add(session& session)
{
	if(this->_members.size() >= 8)
		return false;

	this->_members.push_back(&session);
	return true;
}

bool fb::game::group_system::remove(session& session)
{
	auto i = std::find(this->_members.begin(), this->_members.end(), &session);
	if(i == this->_members.end())
		return false;

	(*i)->group_leave();
	this->_members.erase(i);
	return true;
}
