#include "item.h"
#include "session.h"
#include "map.h"
#include "fb_game.h"

#ifdef small
#undef small
#endif

const fb::game::item::item_limit fb::game::item::DEFAULT_LIMIT;
const fb::game::item fb::game::cash::BRONZE("¿±Àü", 23 + 0xBFFF);
const fb::game::item fb::game::cash::BRONZE_BUNDLE("¿±Àü¹¶Ä¡", 74 + 0xBFFF);
const fb::game::item fb::game::cash::SILVER("ÀºÀü", 70 + 0xBFFF);
const fb::game::item fb::game::cash::SILVER_BUNDLE("ÀºÀü¹¶Ä¡", 73 + 0xBFFF);
const fb::game::item fb::game::cash::GOLD("±ÝÀü", 71 + 0xBFFF);
const fb::game::item fb::game::cash::GOLD_BUNDLE("±Ýµ¢¾î¸®", 72 + 0xBFFF);


fb::game::item::_limit::_limit() : 
	level(0),
	strength(0),
	dexteritry(0),
	intelligence(0),
	cls(0),
	promotion(0),
	sex(fb::game::sex::BOTH)
{
}

fb::game::item::_limit::_limit(uint16_t level, uint16_t strength, uint16_t dexteritry, uint16_t intelligence, uint16_t cls, uint16_t promotion, fb::game::sex sex)
{
}




//
// class item
//

fb::game::item::item() : 
	_core(this),
	_price(0),
	_limit(item::DEFAULT_LIMIT),
	_penalty(penalties::DROP),
	_capacity(1),
	_count(0),
	_trade(true),
	_bundle(true),
	_entrust(0)
{}

fb::game::item::item(const std::string& name, uint16_t look, uint8_t color) : 
	object(0xFFFFFFFF, name, look, color),
	_core(this),
	_price(0),
	_limit(item::DEFAULT_LIMIT),
	_penalty(penalties::DROP),
	_capacity(1),
	_count(0),
	_trade(true),
	_bundle(true),
	_entrust(0)
{}

fb::game::item::item(const fb::game::item& right) : 
	object(right),
	_core(right._core),
	_price(right._price),
	_limit(right._limit),
	_penalty(right._penalty),
	_capacity(right._capacity),
	_count(1),
	_trade(right._trade),
	_bundle(right._bundle),
	_entrust(right._entrust),
	_tooltip(right._tooltip),
	_desc(right._desc),
	_active_script(right._active_script)
{
}

fb::game::item::~item()
{
}

const std::string fb::game::item::name_styled() const
{
	std::stringstream		sstream;
	sstream << this->_name;

	if((this->attr() & item::attrs::ITEM_ATTR_BUNDLE) && this->_count > 1)
		sstream << ' ' << this->_count << "°³";

	return sstream.str();
}

fb::game::item::attrs fb::game::item::attr() const
{
	item::attrs attr = item::attrs::ITEM_ATTR_NONE;
	if(this->_capacity > 1)
		attr = item::attrs(attr | item::attrs::ITEM_ATTR_BUNDLE);
	return attr;
}

fb::game::item* fb::game::item::make() const
{
	return new item(*this);
}


const fb::game::item* fb::game::item::core() const
{
	return this->_core;
}

uint32_t fb::game::item::price() const
{
	return this->_price;
}

void fb::game::item::price(uint32_t value)
{
	this->_price = value;
}

uint16_t fb::game::item::capacity() const
{
	return this->_capacity;
}

void fb::game::item::capacity(uint16_t value)
{
	this->_capacity = value;
}

bool fb::game::item::trade() const
{
	return this->_trade;
}

void fb::game::item::trade(bool value)
{
	this->_trade = value;
}

uint32_t fb::game::item::entrust() const
{
	return this->_entrust;
}

void fb::game::item::entrust(uint32_t value)
{
	this->_entrust = value;
}

const fb::game::item::item_limit& fb::game::item::limit() const
{
	return this->_limit;
}

void fb::game::item::limit(const item::item_limit& value)
{
	this->_limit = value;
}

fb::game::item::penalties fb::game::item::penalty() const
{
	return this->_penalty;
}

void fb::game::item::penalty(penalties value)
{
	this->_penalty = value;
}

const std::string& fb::game::item::desc() const
{
	return this->_desc;
}

void fb::game::item::desc(const std::string& value)
{
	this->_desc = value;
}

const std::string& fb::game::item::active_script() const
{
	return this->_active_script;
}

void fb::game::item::active_script(const std::string& value)
{
	this->_active_script = value;
}

std::string fb::game::item::tip_message() const
{
	std::stringstream		sstream;

	sstream << "°¡°Ý: " << this->_price;
	if(this->_desc.empty() == false)
		sstream << std::endl << std::endl << this->_desc;
	return sstream.str();
}

uint16_t fb::game::item::fill(uint16_t count)
{
	// Ãß°¡ÇÏ°í ³²Àº °¹¼ö ¸®ÅÏ
	uint16_t free_space = this->free_space();
	uint16_t addition = std::min(free_space, count);

	this->_count += addition;
	return std::max(0, count - free_space);
}

uint16_t fb::game::item::reduce(uint16_t count)
{
	// Á¦°ÅÇÏ°í ³²Àº °¹¼ö ¸®ÅÏ
	this->_count -= std::min(count, this->_count);
	return this->_count;
}

uint16_t fb::game::item::free_space() const
{
	return this->_capacity - this->_count;
}

uint16_t fb::game::item::count() const
{
	return this->_count;
}

void fb::game::item::count(uint16_t value)
{
	this->_count = value;
}

bool fb::game::item::empty() const
{
	return this->_count == 0;
}

fb::ostream fb::game::item::make_tip_stream(uint16_t position)
{
	fb::ostream				ostream;
	std::string				message = this->tip_message();

	ostream.write_u8(0x59)
		.write_u16(position)
		.write_u16(message.size())
		.write(message.c_str(), message.size())
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::item::make_trade_show_stream(bool own, uint16_t count) const
{
	fb::ostream				ostream;
	std::stringstream		sstream;

	if(count > 1)
		sstream << this->_name << ' ' << std::to_string(count) << "°³";

	ostream.write_u8(0x42)
		.write_u8(0x02)
		.write_u8(own ? 0x00 : 0x01)
		.write_u8(0x01)
		.write_u16(this->_look)
		.write_u8(this->_color)
		.write_u8(sstream.str().length())
		.write(sstream.str().c_str(), sstream.str().length())
		.write_u8(0x00);

	return ostream;
}

fb::game::item* fb::game::item::handle_drop(object& owner, uint16_t count)
{
	item* item = NULL;
	if((this->attr() & item::attrs::ITEM_ATTR_BUNDLE) && this->_count > count)
	{
		item = this->make();

		count = std::min(this->_count, count);
		item->_count = count;
		this->_count -= count;
	}
	else
	{
		item = this;
	}

	fb::game::map* map = owner.map();
	item->map(map);
	item->position(owner.position());
	return item;
}



//
// class cash
//
fb::game::cash::cash(uint32_t chunk)
{
	this->chunk(chunk);
}

fb::game::cash::~cash()
{
}

const std::string fb::game::cash::name_styled() const
{
	std::stringstream sstream;
	sstream << this->name() << ' ' << this->_chunk << "Àü";

	return sstream.str();
}

fb::game::item::attrs fb::game::cash::attr() const
{
	return item::attrs::ITEM_ATTR_CASH;
}

fb::game::item* fb::game::cash::make() const
{
	return new cash(this->_chunk);
}

uint32_t fb::game::cash::chunk() const
{
	return this->_chunk;
}

void fb::game::cash::chunk(uint32_t value)
{
	this->_chunk = value;

	const fb::game::item* core = NULL;
	if(this->_chunk == 1)
		core = &fb::game::cash::BRONZE;
	else if(this->_chunk < 100)
		core = &fb::game::cash::BRONZE_BUNDLE;
	else if(this->_chunk == 100)
		core = &fb::game::cash::SILVER;
	else if(this->_chunk < 1000)
		core = &fb::game::cash::SILVER_BUNDLE;
	else if(this->_chunk == 1000)
		core = &fb::game::cash::GOLD;
	else
		core = &fb::game::cash::GOLD_BUNDLE;

	this->look(core->look());
	this->color(core->color());
}

uint32_t fb::game::cash::chunk_reduce(uint32_t value)
{
	uint32_t reduce = std::min(this->_chunk, value);

	this->chunk(this->_chunk - reduce);
	return this->_chunk;
}

bool fb::game::cash::empty() const
{
	return this->_chunk == 0;
}



//
// class consume::core
//
fb::game::consume::consume(const std::string& name, uint16_t look, uint8_t color, uint16_t capacity) : 
	item(name, look, color)
{
	this->_capacity = capacity;
}

fb::game::consume::consume(const consume& right) : 
	item(right)
{
	this->_capacity = right._capacity;
}

fb::game::consume::~consume()
{}

fb::game::item::attrs fb::game::consume::attr() const
{
	item::attrs attr = item::attrs::ITEM_ATTR_CONSUME;
	if(this->_capacity > 1)
		attr = item::attrs(attr | item::attrs::ITEM_ATTR_BUNDLE);

	return attr;
}

fb::game::item* fb::game::consume::make() const
{
	return new consume(*this);
}

bool fb::game::consume::handle_acive(session& session)
{
	if(this->_count == 0)
		return false;

	this->_count--;
	return true;
}



//
// class pack::core
//
fb::game::pack::pack(const std::string& name, uint16_t look, uint8_t color, uint16_t durability) : 
	item(name, look, color),
	_durability(durability)
{}

fb::game::pack::pack(const pack& right) : 
	item(right),
	_durability(right._durability)
{}

fb::game::pack::~pack()
{}

fb::game::item::attrs fb::game::pack::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_CONSUME | item::attrs::ITEM_ATTR_PACK);
}

fb::game::item* fb::game::pack::make() const
{
	return new pack(*this);
}

const std::string fb::game::pack::name_styled() const
{
	std::stringstream sstream;
	sstream << this->name() << " [" << this->_durability << " ÀÜ]";

	return sstream.str();
}

bool fb::game::pack::handle_acive(session& session)
{
	if(this->_count == 0)
		return false;

	if(this->_durability == 0)
		return false;

	this->_durability--;
	if(this->_durability == 0)
		this->_count--;

	return true;
}

//
// class equipment
//

fb::game::equipment::equipment(const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	item(name, look, color),
	_dress(dress),
	_durability(1),
	_repairable(true),
	_repair_price(0),
	_rename_price(0),
	_hit(0),
	_damage(0),
	_strength(0),
	_intelligence(0),
	_dexteritry(0),
	_base_hp(0),
	_base_mp(0),
	_hp_percentage(0),
	_mp_percentage(0),
	_healing_cycle(0),
	_defensive(0, 0)
{}

fb::game::equipment::equipment(const equipment& right) : 
	item(right),
	_dress(right._dress),
	_durability(right._durability),
	_repairable(right._repairable),
	_repair_price(right._repair_price),
	_rename_price(right._rename_price),
	_hit(right._hit),
	_damage(right._damage),
	_strength(right._strength),
	_intelligence(right._intelligence),
	_dexteritry(right._dexteritry),
	_base_hp(right._base_hp),
	_base_mp(right._base_mp),
	_hp_percentage(right._hp_percentage),
	_mp_percentage(right._mp_percentage),
	_healing_cycle(right._healing_cycle),
	_defensive(right._defensive)
{}

fb::game::equipment::~equipment()
{}

fb::game::item::attrs fb::game::equipment::attr() const
{
	return item::attrs::ITEM_ATTR_EQUIPMENT;
}

fb::game::item* fb::game::equipment::make() const
{
	return new equipment(*this);
}

uint16_t fb::game::equipment::dress() const
{
	return this->_dress;
}

int16_t fb::game::equipment::defensive_physical() const
{
	return this->_defensive.physical;
}

void fb::game::equipment::defensive_physical(int16_t value)
{
	this->_defensive.physical = value;
}

int16_t fb::game::equipment::defensive_magical() const
{
	return this->_defensive.magical;
}

void fb::game::equipment::defensive_magical(int16_t value)
{
	this->_defensive.magical = value;
}

uint8_t fb::game::equipment::hit() const
{
	return this->_hit;
}

void fb::game::equipment::hit(uint8_t value)
{
	this->_hit = value;
}

uint8_t fb::game::equipment::random_damage() const
{
	return this->_damage;
}

void fb::game::equipment::random_damage(uint8_t value)
{
	this->_damage = value;
}

uint8_t fb::game::equipment::strength() const
{
	return this->_strength;
}

void fb::game::equipment::strength(uint8_t value)
{
	this->_strength = value;
}

uint8_t fb::game::equipment::intelligence() const
{
	return this->_intelligence;
}

void fb::game::equipment::intelligence(uint8_t value)
{
	this->_intelligence = value;
}

uint8_t fb::game::equipment::dexteritry() const
{
	return this->_dexteritry;
}

void fb::game::equipment::dexteritry(uint8_t value)
{
	this->_dexteritry = value;
}

uint32_t fb::game::equipment::base_hp() const
{
	return this->_base_hp;
}

void fb::game::equipment::base_hp(uint32_t value)
{
	this->_base_hp = value;
}

uint32_t fb::game::equipment::base_mp() const
{
	return this->_base_mp;
}

void fb::game::equipment::base_mp(uint32_t value)
{
	this->_base_mp = value;
}

float fb::game::equipment::hp_percentage() const
{
	return this->_hp_percentage;
}

void fb::game::equipment::hp_percentage(float value)
{
	this->_hp_percentage = value;
}

float fb::game::equipment::mp_percentage() const
{
	return this->_mp_percentage;
}

void fb::game::equipment::mp_percentage(float value)
{
	this->_mp_percentage = value;
}

uint8_t fb::game::equipment::healing_cycle() const
{
	return this->_healing_cycle;
}

void fb::game::equipment::healing_cycle(uint8_t value)
{
	this->_healing_cycle = value;
}

uint16_t fb::game::equipment::durability() const
{
	return this->_durability.current;
}

void fb::game::equipment::durability(uint16_t value)
{
	this->_durability.current = value;
}

uint16_t fb::game::equipment::durability_base() const
{
	return this->_durability.base;
}

void fb::game::equipment::durability_base(uint16_t value)
{
	this->_durability.base = value;
}

bool fb::game::equipment::repairable() const
{
	return this->_repairable;
}

void fb::game::equipment::repairable(bool value)
{
	this->_repairable = value;
}

uint32_t fb::game::equipment::repair_price() const
{
	return this->_repair_price;
}

void fb::game::equipment::repair_price(uint32_t value)
{
	this->_repair_price = value;
}

uint32_t fb::game::equipment::rename_price() const
{
	return this->_rename_price;
}

void fb::game::equipment::rename_price(uint32_t value)
{
	this->_rename_price = value;
}

std::string fb::game::equipment::mid_message() const
{
	return std::string();
}

std::string fb::game::equipment::tip_message() const
{
	std::stringstream			sstream;

	sstream << this->_name << std::endl;
	sstream << "³»±¸¼º: " << std::to_string(this->_durability.current) << '/' << std::to_string(this->_durability.base) << ' ' << std::fixed << std::setprecision(1) << (this->_durability.current / (float)this->_durability.base) * 100 << '%' << std::endl;
	sstream << this->mid_message();
	sstream << "¹«Àå:   " << std::to_string(this->_defensive.physical) << " Hit:  " << std::to_string(this->_hit) << " Dam:  " << std::to_string(this->_damage);

	if(this->_base_hp)
		sstream << std::left << std::setw(14) << std::endl << "Ã¼·ÂÄ¡ »ó½Â:" << std::to_string(this->_base_hp);

	if(this->_base_mp)
		sstream << std::left << std::setw(14) << std::endl << "¸¶·ÂÄ¡ »ó½Â:" << std::to_string(this->_base_mp);

	if(this->_strength)
		sstream << std::left << std::setw(14) << std::endl << "Èû »ó½Â:" << std::to_string(this->_strength);

	if(this->_dexteritry)
		sstream << std::left << std::setw(14) << std::endl << "¹ÎÃ¸¼º »ó½Â:" << std::to_string(this->_dexteritry);

	if(this->_intelligence)
		sstream << std::left << std::setw(14) << std::endl << "Áö·Â »ó½Â:" << std::to_string(this->_intelligence);

	if(this->_healing_cycle)
		sstream << std::left << std::setw(14) << std::endl << "Àç»ý·Â »ó½Â:" << std::to_string(this->_healing_cycle);

	std::stringstream class_stream;

	class_stream << std::endl << std::setw(14) << std::left;
	auto limit = this->_limit;
	if(limit.cls == 0)
		class_stream << "Á÷¾÷Á¦ÇÑ¹«";
	else
		class_stream << fb::game::acceptor::instance()->class2name(limit.cls, 0) << "¿ë";
	sstream << class_stream.str() << "·¹º§ " << std::to_string(limit.level) << " ÀÌ»ó";

	const std::string desc = this->_desc;
	if(desc.empty() == false)
		sstream << std::endl << std::endl << desc;

	return sstream.str();
}

fb::ostream fb::game::equipment::make_trade_show_stream(bool own, uint16_t count) const
{
	fb::ostream				ostream;
	std::stringstream		sstream;
	float					percentage = this->_durability.current / float(this->_durability.base) * 100;

	sstream << this->_name << '(' << std::fixed << std::setprecision(1) << percentage << "%)";

	ostream.write_u8(0x42)
		.write_u8(0x02)
		.write_u8(own ? 0x00 : 0x01)
		.write_u8(0x01)
		.write_u16(this->_look)
		.write_u8(this->_color)
		.write_u8(sstream.str().length())
		.write(sstream.str().c_str(), sstream.str().length())
		.write_u8(0x00);

	return ostream;
}


//
// class weapon
//
fb::game::weapon::weapon(const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	equipment(name, look, dress, color),
	_damage_range(range32_t(0, 0), range32_t(0, 0)),
	_sound(0)
{}

fb::game::weapon::weapon(const weapon& right) : 
	equipment(right),
	_damage_range(right._damage_range),
	_sound(right._sound)
{}

fb::game::weapon::~weapon()
{}

fb::game::item::attrs fb::game::weapon::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_WEAPON);
}

fb::game::item* fb::game::weapon::make() const
{
	return new weapon(*this);
}

fb::game::weapon::types fb::game::weapon::weapon_type() const
{
	switch(this->_look / 10000)
	{
	case 0:
		return fb::game::weapon::types::NORMAL;
		
	case 1:
		return fb::game::weapon::types::SPEAR;

	case 2:
		return fb::game::weapon::types::BOW;

	case 3:
		return fb::game::weapon::types::FAN;

	default:
		return fb::game::weapon::types::UNKNOWN;
	}
}

const fb::game::range32_t& fb::game::weapon::damage_small() const
{
	return this->_damage_range.small;
}

void fb::game::weapon::damage_small(uint32_t min, uint32_t max)
{
	this->_damage_range.small.min = min;
	this->_damage_range.small.max = max;
}

const fb::game::range32_t& fb::game::weapon::damage_large() const
{
	return this->_damage_range.large;
}

void fb::game::weapon::damage_large(uint32_t min, uint32_t max)
{
	this->_damage_range.large.min = min;
	this->_damage_range.large.max = max;
}

uint16_t fb::game::weapon::sound() const
{
	return this->_sound;
}

void fb::game::weapon::sound(uint16_t value)
{
	this->_sound = value;
}

const std::string& fb::game::weapon::spell() const
{
	return this->_spell;
}

void fb::game::weapon::spell(const std::string& value)
{
	this->_spell = value;
}

std::string fb::game::weapon::mid_message() const
{
	std::stringstream sstream;

	sstream << "ÆÄ±«·Â: ¡¡¡¡ S:¡¡" << std::to_string(this->_damage_range.small.min) << 'm' << std::to_string(this->_damage_range.small.max) << std::endl;
	sstream << "¡¡¡¡¡¡  ¡¡ ¡¡L:¡¡" << std::to_string(this->_damage_range.large.min) << 'm' << std::to_string(this->_damage_range.large.max) << std::endl;
	return sstream.str();
}

fb::game::armor::armor(const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	equipment(name, look, dress, color)
{
}

fb::game::armor::armor(const armor& right) : 
	equipment(right)
{
}

fb::game::armor::~armor()
{
}

fb::game::item::attrs fb::game::armor::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_ARMOR);
}

fb::game::item* fb::game::armor::make() const
{
	return new armor(*this);
}

fb::game::helmet::helmet(const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	equipment(name, look, dress, color)
{
}

fb::game::helmet::helmet(const helmet& right) : 
	equipment(right)
{
}

fb::game::helmet::~helmet()
{
}

fb::game::item::attrs fb::game::helmet::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_HELMET);
}

fb::game::item* fb::game::helmet::make() const
{
	return new helmet(*this);
}

fb::game::shield::shield(const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	equipment(name, look, dress, color)
{
}

fb::game::shield::shield(const shield& right) : 
	equipment(right)
{
}

fb::game::shield::~shield()
{
}

fb::game::item::attrs fb::game::shield::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_SHIELD);
}

fb::game::item* fb::game::shield::make() const
{
	return new shield(*this);
}

fb::game::ring::ring(const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	equipment(name, look, dress, color)
{
}

fb::game::ring::ring(const ring& right) : 
	equipment(right)
{
}

fb::game::ring::~ring()
{
}

fb::game::item::attrs fb::game::ring::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_RING);
}

fb::game::item* fb::game::ring::make() const
{
	return new ring(*this);
}

fb::game::auxiliary::auxiliary(const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	equipment(name, look, dress, color)
{
}

fb::game::auxiliary::auxiliary(const auxiliary& right) : 
	equipment(right)
{
}

fb::game::auxiliary::~auxiliary()
{
}

fb::game::item::attrs fb::game::auxiliary::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_AUXILIARY);
}

fb::game::item* fb::game::auxiliary::make() const
{
	return new auxiliary(*this);
}

fb::game::arrow::arrow(const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	equipment(name, look, dress, color)
{
}

fb::game::arrow::arrow(const arrow& right) : 
	equipment(right)
{
}

fb::game::arrow::~arrow()
{
}

fb::game::item::attrs fb::game::arrow::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_WEAPON);
}

fb::game::item* fb::game::arrow::make() const
{
	return new arrow(*this);
}

bool fb::game::_itemmix::contains(const item* item) const
{
	for(auto i = this->require.begin(); i != this->require.end(); i++)
	{
		if(item->core() == (*i).item->core() && item->count() >= (*i).count)
			return true;
	}

	return false;
}

void fb::game::_itemmix::require_add(fb::game::item* item, uint32_t count)
{
	this->require.push_back(element(item, count));
}

void fb::game::_itemmix::success_add(fb::game::item* item, uint32_t count)
{
	this->success.push_back(element(item, count));
}

void fb::game::_itemmix::failed_add(fb::game::item* item, uint32_t count)
{
	this->failed.push_back(element(item, count));
}

bool fb::game::_itemmix::matched(const std::vector<item*>& items) const
{
	if(this->require.size() != items.size())
		return false;

	for(auto i = items.cbegin(); i != items.cend(); i++)
	{
		if(this->contains(*i) == false)
			return false;
	}

	return true;
}
