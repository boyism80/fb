#include "item.h"
#include "session.h"
#include "map.h"
#include "fb_game.h"

#ifdef small
#undef small
#endif

const fb::game::item::item_limit fb::game::item::DEFAULT_LIMIT;
const fb::game::cash::core fb::game::cash::BRONZE(0xFFFFFFFF, "¿±Àü", 23 + 0xBFFF);
const fb::game::cash::core fb::game::cash::BRONZE_BUNDLE(0xFFFFFFFF, "¿±Àü¹¶Ä¡", 74 + 0xBFFF);
const fb::game::cash::core fb::game::cash::SILVER(0xFFFFFFFF, "ÀºÀü", 70 + 0xBFFF);
const fb::game::cash::core fb::game::cash::SILVER_BUNDLE(0xFFFFFFFF, "ÀºÀü¹¶Ä¡", 73 + 0xBFFF);
const fb::game::cash::core fb::game::cash::GOLD(0xFFFFFFFF, "±ÝÀü", 71 + 0xBFFF);
const fb::game::cash::core fb::game::cash::GOLD_BUNDLE(0xFFFFFFFF, "±Ýµ¢¾î¸®", 72 + 0xBFFF);


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

fb::game::item::core::core(uint32_t id, const std::string& name, uint16_t look, uint8_t color, uint16_t capacity, const item_limit& limit) : 
	fb::game::object::core(id, name, look, color),
	_price(0),
	_capacity(capacity),
	_limit(limit),
	_penalty(penalties::DROP),
	_trade(true),
	_bundle(false),
	_entrust(0)
{}

fb::game::item::core::core(const fb::game::object::core& core) : 
	fb::game::object::core(core)
{}

fb::game::item::core::~core()
{}

uint32_t fb::game::item::core::price() const
{
	return this->_price;
}

void fb::game::item::core::price(uint32_t value)
{
	this->_price = value;
}

uint16_t fb::game::item::core::capacity() const
{
	return this->_capacity;
}

void fb::game::item::core::capacity(uint16_t value)
{
	this->_capacity = value;
}

bool fb::game::item::core::trade() const
{
	return this->_trade;
}

void fb::game::item::core::trade(bool value)
{
	this->_trade = value;
}

uint32_t fb::game::item::core::entrust() const
{
	return this->_entrust;
}

void fb::game::item::core::entrust(uint32_t value)
{
	this->_entrust = value;
}

const fb::game::item::item_limit& fb::game::item::core::limit() const
{
	return this->_limit;
}

void fb::game::item::core::limit(const item::item_limit& value)
{
	this->_limit = value;
}

fb::game::item::penalties fb::game::item::core::penalty() const
{
	return this->_penalty;
}

void fb::game::item::core::penalty(penalties value)
{
	this->_penalty = value;
}

const std::string& fb::game::item::core::desc() const
{
	return this->_desc;
}

void fb::game::item::core::desc(const std::string& value)
{
	this->_desc = value;
}

const std::string& fb::game::item::core::active_script() const
{
	return this->_active_script;
}

void fb::game::item::core::active_script(const std::string& value)
{
	this->_active_script = value;
}

fb::game::item::attrs fb::game::item::core::attr() const
{
	item::attrs attr = item::attrs::ITEM_ATTR_NONE;
	if(this->capacity() > 1)
		attr = item::attrs(attr | item::attrs::ITEM_ATTR_BUNDLE);
	return attr;
}

fb::game::object* fb::game::item::core::make() const
{
	return new item(this);
}






//
// class item
//

fb::game::item::item(const fb::game::item::core* core) : 
	fb::game::object(core),
	_count(1)
{}

fb::game::item::item(const fb::game::item& right) : 
	object(right),
	_count(right._count)
{
}

fb::game::item::~item()
{
}

const std::string fb::game::item::name_styled() const
{
	std::stringstream		sstream;
	sstream << this->name();

	if((this->attr() & item::attrs::ITEM_ATTR_BUNDLE) && this->_count > 1)
		sstream << ' ' << this->_count << "°³";

	return sstream.str();
}

const std::string fb::game::item::name_trade() const
{
	return this->name_styled();
}


std::string fb::game::item::tip_message() const
{
	std::stringstream		sstream;

	sstream << "°¡°Ý: " << this->price();
	const std::string& desc = this->desc();
	if(desc.empty() == false)
		sstream << std::endl << std::endl << desc;
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
	return this->capacity() - this->_count;
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

uint32_t fb::game::item::price() const
{
	return static_cast<const core*>(this->_core)->_price;
}

uint16_t fb::game::item::capacity() const
{
	return static_cast<const core*>(this->_core)->_capacity;
}

bool fb::game::item::trade() const
{
	return static_cast<const core*>(this->_core)->_trade;
}

uint32_t fb::game::item::entrust() const
{
	return static_cast<const core*>(this->_core)->_entrust;
}

const fb::game::item::item_limit& fb::game::item::limit() const
{
	return static_cast<const core*>(this->_core)->_limit;
}

fb::game::item::penalties fb::game::item::penalty() const
{
	return static_cast<const core*>(this->_core)->_penalty;
}

const std::string& fb::game::item::desc() const
{
	return static_cast<const core*>(this->_core)->_desc;
}

const std::string& fb::game::item::active_script() const
{
	return static_cast<const core*>(this->_core)->_active_script;
}

fb::game::item::attrs fb::game::item::attr() const
{
	return static_cast<const core*>(this->_core)->attr();
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

fb::game::item* fb::game::item::handle_drop(object& owner, uint16_t count)
{
	item* item = NULL;
	if((this->attr() & item::attrs::ITEM_ATTR_BUNDLE) && this->_count > count)
	{
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



fb::game::cash::core::core(uint32_t id, const std::string& name, uint16_t look, uint8_t color) : 
	fb::game::item::core(id, name, look, color)
{}

fb::game::cash::core::~core()
{}

fb::game::item::attrs fb::game::cash::core::attr() const
{
	return item::attrs::ITEM_ATTR_CASH;
}

fb::game::object* fb::game::cash::core::make() const
{
	return new cash(0);
}



//
// class cash
//
fb::game::cash::cash(uint32_t chunk) : 
	fb::game::item(NULL)
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

uint32_t fb::game::cash::chunk() const
{
	return this->_chunk;
}

void fb::game::cash::chunk(uint32_t value)
{
	this->_chunk = value;

	const fb::game::item* core = NULL;
	if(this->_chunk == 1)
		this->_core = &fb::game::cash::BRONZE;
	else if(this->_chunk < 100)
		this->_core = &fb::game::cash::BRONZE_BUNDLE;
	else if(this->_chunk == 100)
		this->_core = &fb::game::cash::SILVER;
	else if(this->_chunk < 1000)
		this->_core = &fb::game::cash::SILVER_BUNDLE;
	else if(this->_chunk == 1000)
		this->_core = &fb::game::cash::GOLD;
	else
		this->_core = &fb::game::cash::GOLD_BUNDLE;
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
fb::game::consume::core::core(uint32_t id, const std::string& name, uint16_t look, uint8_t color, uint16_t capacity) : 
	fb::game::item::core(id, name, look, color, capacity)
{}

fb::game::consume::core::~core()
{}

fb::game::item::attrs fb::game::consume::core::attr() const
{
	item::attrs attr = item::attrs::ITEM_ATTR_CONSUME;
	if(this->_capacity > 1)
		attr = item::attrs(attr | item::attrs::ITEM_ATTR_BUNDLE);

	return attr;
}

fb::game::object* fb::game::consume::core::make() const
{
	return new consume(this);
}



fb::game::consume::consume(const fb::game::consume::core* core) : 
	fb::game::item(core)
{}

fb::game::consume::consume(const consume& right) : 
	fb::game::item(right)
{
	
}

fb::game::consume::~consume()
{}

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
fb::game::pack::core::core(uint32_t id, const std::string& name, uint16_t look, uint8_t color, uint16_t durability) : 
	fb::game::item::core(id, name, look, color),
	_durability(durability)
{}

fb::game::pack::core::~core()
{}

fb::game::item::attrs fb::game::pack::core::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_CONSUME | item::attrs::ITEM_ATTR_PACK);
}

fb::game::object* fb::game::pack::core::make() const
{
	return new pack(this);
}

fb::game::pack::pack(const fb::game::pack::core* core) : 
	fb::game::item(core)
{
	this->_durability = this->base_durability();
}

fb::game::pack::pack(const pack& right) : 
	item(right),
	_durability(right._durability)
{}

fb::game::pack::~pack()
{}


uint16_t fb::game::pack::durability() const
{
	return this->_durability;
}

uint16_t fb::game::pack::base_durability() const
{
	return static_cast<const core*>(this->_core)->_durability;
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

fb::game::equipment::core::core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color, uint16_t durability) : 
	fb::game::item::core(id, name, look, color),
	_dress(dress),
	_durability(durability)
{}

fb::game::equipment::core::~core()
{}

fb::game::item::attrs fb::game::equipment::core::attr() const
{
	return item::attrs::ITEM_ATTR_EQUIPMENT;
}

fb::game::object* fb::game::equipment::core::make() const
{
	return new equipment(this);
}

uint16_t fb::game::equipment::core::dress() const
{
	return this->_dress;
}

int16_t fb::game::equipment::core::defensive_physical() const
{
	return this->_defensive.physical;
}

void fb::game::equipment::core::defensive_physical(int16_t value)
{
	this->_defensive.physical = value;
}

int16_t fb::game::equipment::core::defensive_magical() const
{
	return this->_defensive.magical;
}

void fb::game::equipment::core::defensive_magical(int16_t value)
{
	this->_defensive.magical = value;
}

uint8_t fb::game::equipment::core::hit() const
{
	return this->_hit;
}

void fb::game::equipment::core::hit(uint8_t value)
{
	this->_hit = value;
}

uint8_t fb::game::equipment::core::damage() const
{
	return this->_damage;
}

void fb::game::equipment::core::damage(uint8_t value)
{
	this->_damage = value;
}

uint8_t fb::game::equipment::core::strength() const
{
	return this->_strength;
}

void fb::game::equipment::core::strength(uint8_t value)
{
	this->_strength = value;
}

uint8_t fb::game::equipment::core::intelligence() const
{
	return this->_intelligence;
}

void fb::game::equipment::core::intelligence(uint8_t value)
{
	this->_intelligence = value;
}

uint8_t fb::game::equipment::core::dexteritry() const
{
	return this->_dexteritry;
}

void fb::game::equipment::core::dexteritry(uint8_t value)
{
	this->_dexteritry = value;
}

uint32_t fb::game::equipment::core::base_hp() const
{
	return this->_base_hp;
}

void fb::game::equipment::core::base_hp(uint32_t value)
{
	this->_base_hp = value;
}

uint32_t fb::game::equipment::core::base_mp() const
{
	return this->_base_mp;
}

void fb::game::equipment::core::base_mp(uint32_t value)
{
	this->_base_mp = value;
}

float fb::game::equipment::core::hp_percentage() const
{
	return this->_hp_percentage;
}

void fb::game::equipment::core::hp_percentage(float value)
{
	this->_hp_percentage = value;
}

float fb::game::equipment::core::mp_percentage() const
{
	return this->_mp_percentage;
}

void fb::game::equipment::core::mp_percentage(float value)
{
	this->_mp_percentage = value;
}

uint8_t fb::game::equipment::core::healing_cycle() const
{
	return this->_healing_cycle;
}

void fb::game::equipment::core::healing_cycle(uint8_t value)
{
	this->_healing_cycle = value;
}

uint16_t fb::game::equipment::core::durability() const
{
	return this->_durability;
}

void fb::game::equipment::core::durability(uint16_t value)
{
	this->_durability = value;
}

bool fb::game::equipment::core::repairable() const
{
	return this->_repairable;
}

void fb::game::equipment::core::repairable(bool value)
{
	this->_repairable = value;
}

uint32_t fb::game::equipment::core::repair_price() const
{
	return this->_repair_price;
}

void fb::game::equipment::core::repair_price(uint32_t value)
{
	this->_repair_price = value;
}

uint32_t fb::game::equipment::core::rename_price() const
{
	return this->_rename_price;
}

void fb::game::equipment::core::rename_price(uint32_t value)
{
	this->_rename_price = value;
}

fb::game::equipment::equipment(const equipment::core* core) : 
	item(core)
{
	this->_durability = static_cast<const equipment::core*>(this->_core)->_durability;
}

fb::game::equipment::equipment(const equipment& right) :
	item(right)
{}

fb::game::equipment::~equipment()
{}


const std::string fb::game::equipment::name_trade() const
{
	std::stringstream sstream;
	float					percentage = this->durability() / float(this->durability_base()) * 100;
	sstream << this->name() << '(' << std::fixed << std::setprecision(1) << percentage << "%)";

	return sstream.str();
}

uint16_t fb::game::equipment::dress() const
{
	return static_cast<const core*>(this->_core)->_dress;
}

uint16_t fb::game::equipment::durability() const
{
	return this->_durability;
}

uint16_t fb::game::equipment::durability_base() const
{
	return static_cast<const core*>(this->_core)->_durability;
}

bool fb::game::equipment::repairable() const
{
	return static_cast<const core*>(this->_core)->_repairable;
}

uint32_t fb::game::equipment::repair_price() const
{
	return static_cast<const core*>(this->_core)->_repair_price;
}

uint32_t fb::game::equipment::rename_price() const
{
	return static_cast<const core*>(this->_core)->_rename_price;
}

int16_t fb::game::equipment::defensive_physical() const
{
	return static_cast<const core*>(this->_core)->_defensive.physical;
}

int16_t fb::game::equipment::defensive_magical() const
{
	return static_cast<const core*>(this->_core)->_defensive.magical;
}

uint8_t fb::game::equipment::hit() const
{
	return static_cast<const core*>(this->_core)->_hit;
}

uint8_t fb::game::equipment::damage() const
{
	return static_cast<const core*>(this->_core)->_damage;
}

uint8_t fb::game::equipment::strength() const
{
	return static_cast<const core*>(this->_core)->_strength;
}

uint8_t fb::game::equipment::intelligence() const
{
	return static_cast<const core*>(this->_core)->_intelligence;
}

uint8_t fb::game::equipment::dexteritry() const
{
	return static_cast<const core*>(this->_core)->_dexteritry;
}

uint32_t fb::game::equipment::base_hp() const
{
	return static_cast<const core*>(this->_core)->_base_hp;
}

uint32_t fb::game::equipment::base_mp() const
{
	return static_cast<const core*>(this->_core)->_base_mp;
}

float fb::game::equipment::hp_percentage() const
{
	return static_cast<const core*>(this->_core)->_hp_percentage;
}

float fb::game::equipment::mp_percentage() const
{
	return static_cast<const core*>(this->_core)->_mp_percentage;
}

uint8_t fb::game::equipment::healing_cycle() const
{
	return static_cast<const core*>(this->_core)->_healing_cycle;
}

std::string fb::game::equipment::mid_message() const
{
	return std::string();
}

std::string fb::game::equipment::tip_message() const
{
	std::stringstream			sstream;

	sstream << this->name() << std::endl;
	sstream << "³»±¸¼º: " << std::to_string(this->durability()) << '/' << std::to_string(this->durability_base()) << ' ' << std::fixed << std::setprecision(1) << (this->durability() / (float)this->durability_base()) * 100 << '%' << std::endl;
	sstream << this->mid_message();
	sstream << "¹«Àå:   " << std::to_string(this->defensive_physical()) << " Hit:  " << std::to_string(this->hit()) << " Dam:  " << std::to_string(this->damage());

	if(this->base_hp())
		sstream << std::left << std::setw(14) << std::endl << "Ã¼·ÂÄ¡ »ó½Â:" << std::to_string(this->base_hp());

	if(this->base_mp())
		sstream << std::left << std::setw(14) << std::endl << "¸¶·ÂÄ¡ »ó½Â:" << std::to_string(this->base_mp());

	if(this->strength())
		sstream << std::left << std::setw(14) << std::endl << "Èû »ó½Â:" << std::to_string(this->strength());

	if(this->dexteritry())
		sstream << std::left << std::setw(14) << std::endl << "¹ÎÃ¸¼º »ó½Â:" << std::to_string(this->dexteritry());

	if(this->intelligence())
		sstream << std::left << std::setw(14) << std::endl << "Áö·Â »ó½Â:" << std::to_string(this->intelligence());

	if(this->healing_cycle())
		sstream << std::left << std::setw(14) << std::endl << "Àç»ý·Â »ó½Â:" << std::to_string(this->healing_cycle());

	std::stringstream class_stream;

	class_stream << std::endl << std::setw(14) << std::left;
	auto limit = this->limit();
	if(limit.cls == 0)
		class_stream << "Á÷¾÷Á¦ÇÑ¹«";
	else
		class_stream << fb::game::acceptor::instance()->class2name(limit.cls, 0) << "¿ë";
	sstream << class_stream.str() << "·¹º§ " << std::to_string(limit.level) << " ÀÌ»ó";

	const std::string desc = this->desc();
	if(desc.empty() == false)
		sstream << std::endl << std::endl << desc;

	return sstream.str();
}


//
// class weapon
//
fb::game::weapon::core::core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	fb::game::equipment::core(id, name, look, dress, color),
	_damage_range(range32_t(0, 0), range32_t(0, 0))
{}

fb::game::weapon::core::core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color, const range32_t& small, const range32_t& large) : 
	fb::game::equipment::core(id, name, look, dress, color),
	_damage_range(small, large)
{
}

fb::game::weapon::core::~core()
{}

fb::game::item::attrs fb::game::weapon::core::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_WEAPON);
}

fb::game::object* fb::game::weapon::core::make() const
{
	return new weapon(this);
}

fb::game::weapon::types fb::game::weapon::core::weapon_type() const
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

const fb::game::range32_t& fb::game::weapon::core::damage_small() const
{
	return this->_damage_range.small;
}

void fb::game::weapon::core::damage_small(uint32_t min, uint32_t max)
{
	this->_damage_range.small.min = min;
	this->_damage_range.small.max = max;
}

const fb::game::range32_t& fb::game::weapon::core::damage_large() const
{
	return this->_damage_range.large;
}

void fb::game::weapon::core::damage_large(uint32_t min, uint32_t max)
{
	this->_damage_range.large.min = min;
	this->_damage_range.large.max = max;
}

uint16_t fb::game::weapon::core::sound() const
{
	return this->_sound;
}

void fb::game::weapon::core::sound(uint16_t value)
{
	this->_sound = value;
}

const std::string& fb::game::weapon::core::spell() const
{
	return this->_spell;
}

void fb::game::weapon::core::spell(const std::string& value)
{
	this->_spell = value;
}


fb::game::weapon::weapon(const core* core) : 
	equipment(core)
{}

fb::game::weapon::weapon(const weapon& right) : 
	equipment(right)
{}

fb::game::weapon::~weapon()
{}


fb::game::weapon::types fb::game::weapon::weapon_type() const
{
	return static_cast<const core*>(this->_core)->weapon_type();
}

const fb::game::range32_t& fb::game::weapon::damage_small() const
{
	return static_cast<const core*>(this->_core)->_damage_range.small;
}

const fb::game::range32_t& fb::game::weapon::damage_large() const
{
	return static_cast<const core*>(this->_core)->_damage_range.large;
}

uint16_t fb::game::weapon::sound() const
{
	return static_cast<const core*>(this->_core)->_sound;
}

const std::string& fb::game::weapon::spell() const
{
	return static_cast<const core*>(this->_core)->_spell;
}

std::string fb::game::weapon::mid_message() const
{
	std::stringstream sstream;

	sstream << "ÆÄ±«·Â: ¡¡¡¡ S:¡¡" << std::to_string(this->damage_small().min) << 'm' << std::to_string(this->damage_small().max) << std::endl;
	sstream << "¡¡¡¡¡¡  ¡¡ ¡¡L:¡¡" << std::to_string(this->damage_large().min) << 'm' << std::to_string(this->damage_large().max) << std::endl;
	return sstream.str();
}


//
// armor::core methods
//
fb::game::armor::core::core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	fb::game::equipment::core(id, name, look, dress, color)
{}

fb::game::armor::core::~core()
{}

fb::game::item::attrs fb::game::armor::core::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_ARMOR);
}

fb::game::object* fb::game::armor::core::make() const
{
	return new armor(this);
}



//
// armor methods
//
fb::game::armor::armor(const core* core) : 
	equipment(core)
{
}

fb::game::armor::armor(const armor& right) : 
	equipment(right)
{
}

fb::game::armor::~armor()
{
}



//
// helmet::core methods
//
fb::game::helmet::core::core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	fb::game::equipment::core(id, name, look, dress, color)
{}

fb::game::helmet::core::~core()
{}

fb::game::item::attrs fb::game::helmet::core::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_HELMET);
}

fb::game::object* fb::game::helmet::core::make() const
{
	return new helmet(this);
}



//
// helmet methods
//
fb::game::helmet::helmet(const core* core) : 
	equipment(core)
{
}

fb::game::helmet::helmet(const helmet& right) : 
	equipment(right)
{
}

fb::game::helmet::~helmet()
{
}



//
// shield::core methods
//
fb::game::shield::core::core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	fb::game::equipment::core(id, name, look, dress, color)
{}

fb::game::shield::core::~core()
{}

fb::game::item::attrs fb::game::shield::core::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_SHIELD);
}

fb::game::object* fb::game::shield::core::make() const
{
	return new shield(this);
}



//
// shield methods
//
fb::game::shield::shield(const core* core) : 
	equipment(core)
{
}

fb::game::shield::shield(const shield& right) : 
	equipment(right)
{
}

fb::game::shield::~shield()
{
}



//
// ring::core methods
//
fb::game::ring::core::core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	fb::game::equipment::core(id, name, look, dress, color)
{}

fb::game::ring::core::~core()
{}

fb::game::item::attrs fb::game::ring::core::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_RING);
}

fb::game::object* fb::game::ring::core::make() const
{
	return new ring(this);
}



//
// ring methods
//
fb::game::ring::ring(const core* core) : 
	equipment(core)
{
}

fb::game::ring::ring(const ring& right) : 
	equipment(right)
{
}

fb::game::ring::~ring()
{
}



//
// auxiliary::core methods
//
fb::game::auxiliary::core::core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	fb::game::equipment::core(id, name, look, dress, color)
{}

fb::game::auxiliary::core::~core()
{}

fb::game::item::attrs fb::game::auxiliary::core::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_AUXILIARY);
}

fb::game::object* fb::game::auxiliary::core::make() const
{
	return new auxiliary(this);
}



//
// auxiliary methods
//
fb::game::auxiliary::auxiliary(const core* core) : 
	equipment(core)
{
}

fb::game::auxiliary::auxiliary(const auxiliary& right) : 
	equipment(right)
{
}

fb::game::auxiliary::~auxiliary()
{
}



//
// arrow::core methods
//
fb::game::arrow::core::core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
	fb::game::equipment::core(id, name, look, dress, color)
{}

fb::game::arrow::core::~core()
{}

fb::game::item::attrs fb::game::arrow::core::attr() const
{
	return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT);
}

fb::game::object* fb::game::arrow::core::make() const
{
	return new arrow(this);
}



//
// arrow methods
//


fb::game::arrow::arrow(const core* core) : 
	equipment(core)
{
}

fb::game::arrow::arrow(const arrow& right) : 
	equipment(right)
{
}

fb::game::arrow::~arrow()
{
}


bool fb::game::_itemmix::contains(const item* item) const
{
	for(auto i : this->require)
	{
		if(item->based() == i.item && item->count() >= i.count)
			return true;
	}

	return false;
}

void fb::game::_itemmix::require_add(fb::game::item::core* item, uint32_t count)
{
	this->require.push_back(element(item, count));
}

void fb::game::_itemmix::success_add(fb::game::item::core* item, uint32_t count)
{
	this->success.push_back(element(item, count));
}

void fb::game::_itemmix::failed_add(fb::game::item::core* item, uint32_t count)
{
	this->failed.push_back(element(item, count));
}

bool fb::game::_itemmix::matched(const std::vector<item*>& items) const
{
	if(this->require.size() != items.size())
		return false;

	for(const auto i : items)
	{
		if(this->contains(i) == false)
			return false;
	}

	return true;
}
