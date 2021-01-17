#include "model/session/session.h"
#include "model/acceptor/acceptor.game.h"
#include "model/map/map.h"
#include "model/table/table.game.h"
#include "item.h"

#ifdef small
#undef small
#endif

const fb::game::item::item_limit fb::game::item::DEFAULT_LIMIT;
const fb::game::cash::master fb::game::cash::BRONZE(0xFFFFFFFF, "엽전", 23 + 0xBFFF);
const fb::game::cash::master fb::game::cash::BRONZE_BUNDLE(0xFFFFFFFF, "엽전뭉치", 74 + 0xBFFF);
const fb::game::cash::master fb::game::cash::SILVER(0xFFFFFFFF, "은전", 70 + 0xBFFF);
const fb::game::cash::master fb::game::cash::SILVER_BUNDLE(0xFFFFFFFF, "은전뭉치", 73 + 0xBFFF);
const fb::game::cash::master fb::game::cash::GOLD(0xFFFFFFFF, "금전", 71 + 0xBFFF);
const fb::game::cash::master fb::game::cash::GOLD_BUNDLE(0xFFFFFFFF, "금덩어리", 72 + 0xBFFF);


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

fb::game::item::_limit::_limit(const _limit& right) : 
    level(right.level),
    strength(right.strength),
    dexteritry(right.dexteritry),
    intelligence(right.intelligence),
    cls(right.cls),
    promotion(right.promotion),
    sex(right.sex)
{
}

fb::game::item::master::master(uint32_t id, const std::string& name, uint16_t look, uint8_t color, uint16_t capacity, const item_limit& limit) : 
    fb::game::object::master(name, look, color),
    _id(id),
    _price(0),
    _capacity(capacity),
    _limit(limit),
    _penalty(penalties::DROP),
    _trade(true),
    _bundle(false),
    _entrust(0)
{}

fb::game::item::master::master(const fb::game::object::master& master) : 
    fb::game::object::master(master)
{}

fb::game::item::master::~master()
{}

uint32_t fb::game::item::master::id() const
{
    return this->_id;
}

void fb::game::item::master::id(uint32_t id)
{
    this->_id = id;
}

uint32_t fb::game::item::master::price() const
{
    return this->_price;
}

void fb::game::item::master::price(uint32_t value)
{
    this->_price = value;
}

uint16_t fb::game::item::master::capacity() const
{
    return this->_capacity;
}

void fb::game::item::master::capacity(uint16_t value)
{
    this->_capacity = value;
}

bool fb::game::item::master::trade() const
{
    return this->_trade._enabled;
}

void fb::game::item::master::trade(bool value)
{
    this->_trade._enabled = value;
}

bool fb::game::item::master::entrust_enabled() const
{
    return this->_entrust._enabled;
}

void fb::game::item::master::entrust_enabled(bool value)
{
    this->_entrust._enabled = value;
}

uint32_t fb::game::item::master::entrust_price() const
{
    return this->_entrust._price;
}

void fb::game::item::master::entrust_price(uint32_t value)
{
    this->_entrust._price = value;
}

const fb::game::item::item_limit& fb::game::item::master::limit() const
{
    return this->_limit;
}

void fb::game::item::master::limit(const item::item_limit& value)
{
    this->_limit = value;
}

fb::game::item::penalties fb::game::item::master::penalty() const
{
    return this->_penalty;
}

void fb::game::item::master::penalty(penalties value)
{
    this->_penalty = value;
}

const std::string& fb::game::item::master::desc() const
{
    return this->_desc;
}

void fb::game::item::master::desc(const std::string& value)
{
    this->_desc = value;
}

const std::string& fb::game::item::master::active_script() const
{
    return this->_active_script;
}

void fb::game::item::master::active_script(const std::string& value)
{
    this->_active_script = value;
}

int fb::game::item::master::builtin_make(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto master = thread->touserdata<fb::game::item::master>(1);
    auto object = master->make(acceptor);

    auto map = thread->touserdata<fb::game::map>(2);
    object->map(map);

    if(lua_istable(lua, 3))
    {
        lua_rawgeti(lua, 3, 1);
        object->x((uint16_t)thread->tointeger(-1));
        lua_remove(lua, -1);

        lua_rawgeti(lua, 3, 2);
        object->y((uint16_t)thread->tointeger(-1));
        lua_remove(lua, -1);
    }
    else
    {
        object->position((uint16_t)thread->tointeger(3), (uint16_t)thread->tointeger(4));
    }

    acceptor->send(*object, fb::protocol::game::response::object::show(*object), acceptor::scope::PIVOT);

    object->to_lua(lua);
    return 1;
}

fb::game::item::attrs fb::game::item::master::attr() const
{
    auto                    attr = attrs::ITEM_ATTR_NONE;
    if(this->capacity() > 1)
        attr = attrs(attr | attrs::ITEM_ATTR_BUNDLE);
    return attr;
}

fb::game::object* fb::game::item::master::make(listener* listener) const
{
    return new item(this, listener);
}






//
// class item
//

fb::game::item::item(const fb::game::item::master* master, listener* listener) : 
    fb::game::object(master, listener),
    _id(0xFFFFFFFF),
    _owner(nullptr),
    _listener(listener),
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
    std::stringstream       sstream;
    sstream << this->name();

    if((this->attr() & item::attrs::ITEM_ATTR_BUNDLE) && this->_count > 1)
        sstream << ' ' << this->_count << "개";

    return sstream.str();
}

const std::string fb::game::item::name_trade() const
{
    return this->name_styled();
}


std::string fb::game::item::tip_message() const
{
    std::stringstream       sstream;

    sstream << "가격: " << this->price();
    const std::string& desc = this->desc();
    if(desc.empty() == false)
        sstream << std::endl << std::endl << desc;
    return sstream.str();
}

uint32_t fb::game::item::id() const
{
    return this->_id;
}

void fb::game::item::id(uint32_t id)
{
    this->_id = id;
}

uint16_t fb::game::item::fill(uint16_t count)
{
    // 추가하고 남은 갯수 리턴
    uint16_t free_space = this->free_space();
    uint16_t addition = std::min(free_space, count);

    this->_count += addition;
    return std::max(0, count - free_space);
}

uint16_t fb::game::item::reduce(uint16_t count)
{
    count = std::min(this->_count, count);
    this->_count -= count;
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
    return static_cast<const master*>(this->_master)->_price;
}

uint16_t fb::game::item::capacity() const
{
    return static_cast<const master*>(this->_master)->_capacity;
}

bool fb::game::item::unique() const
{
    return !static_cast<const master*>(this->_master)->_trade._enabled;
}

bool fb::game::item::entrust_enabled() const
{
    return static_cast<const master*>(this->_master)->_entrust._enabled;
}

uint32_t fb::game::item::entrust_price() const
{
    return static_cast<const master*>(this->_master)->_entrust._price;
}

const fb::game::item::item_limit& fb::game::item::limit() const
{
    return static_cast<const master*>(this->_master)->_limit;
}

fb::game::item::penalties fb::game::item::penalty() const
{
    return static_cast<const master*>(this->_master)->_penalty;
}

const std::string& fb::game::item::desc() const
{
    return static_cast<const master*>(this->_master)->_desc;
}

const std::string& fb::game::item::active_script() const
{
    return static_cast<const master*>(this->_master)->_active_script;
}

fb::game::item::attrs fb::game::item::attr() const
{
    return static_cast<const master*>(this->_master)->attr();
}

bool fb::game::item::active()
{
    if(this->empty())
        this->_owner->items.remove(*this);

    return false;
}

fb::game::item* fb::game::item::split(uint16_t count)
{
    if(this->unique())
        throw std::runtime_error(message::exception::CANNOT_DROP_ITEM);

    count = std::min(count, this->count());

    item* item = nullptr;
    if((this->attr() & item::attrs::ITEM_ATTR_BUNDLE) && this->_count > count)
    {
        this->_count -= count;
        
        item = this->based<fb::game::item::master>()->make<fb::game::item>(this->_listener);
        item->_count = count;
    }
    else
    {
        item = this;
    }

    item->id(0xFFFFFFFF);
    return item;
}



fb::game::cash::master::master(uint32_t id, const std::string& name, uint16_t look, uint8_t color) : 
    fb::game::item::master(id, name, look, color)
{}

fb::game::cash::master::~master()
{}

fb::game::item::attrs fb::game::cash::master::attr() const
{
    return item::attrs::ITEM_ATTR_CASH;
}

fb::game::object* fb::game::cash::master::make(listener* listener) const
{
    return new cash(0, listener);
}



//
// class cash
//
fb::game::cash::cash(uint32_t chunk, listener* listener) : 
    fb::game::item(nullptr, listener)
{
    this->chunk(chunk);
}

fb::game::cash::~cash()
{
}

const std::string fb::game::cash::name_styled() const
{
    std::stringstream sstream;
    sstream << this->name() << ' ' << this->_chunk << "전";

    return sstream.str();
}

uint32_t fb::game::cash::chunk() const
{
    return this->_chunk;
}

fb::game::cash* fb::game::cash::chunk(uint32_t value)
{
    this->_chunk = value;

    const fb::game::item* master = nullptr;
    if(this->_chunk == 1)
        this->_master = &fb::game::cash::BRONZE;
    else if(this->_chunk < 100)
        this->_master = &fb::game::cash::BRONZE_BUNDLE;
    else if(this->_chunk == 100)
        this->_master = &fb::game::cash::SILVER;
    else if(this->_chunk < 1000)
        this->_master = &fb::game::cash::SILVER_BUNDLE;
    else if(this->_chunk == 1000)
        this->_master = &fb::game::cash::GOLD;
    else
        this->_master = &fb::game::cash::GOLD_BUNDLE;

    if(this->_chunk == 0)
    {
        if(this->_map != nullptr)
            this->map(nullptr);

        delete this;
        return nullptr;
    }
    else
    {
        if(this->_listener != nullptr)
            this->_listener->on_show(*this, true);

        return this;
    }
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
// class consume::master
//
fb::game::consume::master::master(uint32_t id, const std::string& name, uint16_t look, uint8_t color, uint16_t capacity) : 
    fb::game::item::master(id, name, look, color, capacity)
{}

fb::game::consume::master::~master()
{}

fb::game::item::attrs fb::game::consume::master::attr() const
{
    item::attrs attr = item::attrs::ITEM_ATTR_CONSUME;
    if(this->_capacity > 1)
        attr = item::attrs(attr | item::attrs::ITEM_ATTR_BUNDLE);

    return attr;
}

fb::game::object* fb::game::consume::master::make(listener* listener) const
{
    return new consume(this, listener);
}



fb::game::consume::consume(const fb::game::consume::master* master, listener* listener) : 
    fb::game::item(master, listener)
{
    this->_count = 10;
}

fb::game::consume::consume(const consume& right) : 
    fb::game::item(right)
{

}

fb::game::consume::~consume()
{}

bool fb::game::consume::active()
{
    if(this->_count == 0)
        return false;
    
    this->_count--;
    if(this->_listener != nullptr)
    {
        this->_listener->on_item_update(*this->_owner, this->_owner->items.to_index(*this));
        this->_listener->on_action(*this->_owner, fb::game::action::EAT, fb::game::duration::DURATION_EAT, fb::game::sound::EAT);
    }

    if(this->empty())
        this->_owner->items.remove(*this, -1, item::delete_attr::DELETE_EAT);
    return true;
}



//
// class pack::master
//
fb::game::pack::master::master(uint32_t id, const std::string& name, uint16_t look, uint8_t color, uint16_t durability) : 
    fb::game::item::master(id, name, look, color),
    _durability(durability)
{}

fb::game::pack::master::~master()
{}

fb::game::item::attrs fb::game::pack::master::attr() const
{
    return item::attrs(item::attrs::ITEM_ATTR_CONSUME | item::attrs::ITEM_ATTR_PACK);
}

fb::game::object* fb::game::pack::master::make(listener* listener) const
{
    return new pack(this, listener);
}

fb::game::pack::pack(const fb::game::pack::master* master, listener* listener) : 
    fb::game::item(master, listener)
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
    return static_cast<const master*>(this->_master)->_durability;
}

const std::string fb::game::pack::name_styled() const
{
    std::stringstream sstream;
    sstream << this->name() << " [" << this->_durability << " 잔]";

    return sstream.str();
}

bool fb::game::pack::active()
{
    if(this->_durability <= 0)
        return false;

    this->_durability--;
    if(this->_durability <= 0)
        this->_count = 0;
    
    if(this->_listener != nullptr)
        this->_listener->on_item_update(*this->_owner, this->_owner->items.to_index(*this));

    if(this->empty())
        this->_owner->items.remove(*this, -1, item::delete_attr::DELETE_REDUCE);

    return true;
}

//
// class equipment
//

fb::game::equipment::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color, uint16_t durability) : 
    fb::game::item::master(id, name, look, color),
    _dress(dress),
    _durability(durability)
{}

fb::game::equipment::master::~master()
{}

fb::game::item::attrs fb::game::equipment::master::attr() const
{
    return item::attrs::ITEM_ATTR_EQUIPMENT;
}

fb::game::object* fb::game::equipment::master::make(listener* listener) const
{
    return new equipment(this, listener);
}

uint16_t fb::game::equipment::master::dress() const
{
    return this->_dress;
}

int16_t fb::game::equipment::master::defensive_physical() const
{
    return this->_defensive.physical;
}

void fb::game::equipment::master::defensive_physical(int16_t value)
{
    this->_defensive.physical = value;
}

int16_t fb::game::equipment::master::defensive_magical() const
{
    return this->_defensive.magical;
}

void fb::game::equipment::master::defensive_magical(int16_t value)
{
    this->_defensive.magical = value;
}

uint8_t fb::game::equipment::master::hit() const
{
    return this->_hit;
}

void fb::game::equipment::master::hit(uint8_t value)
{
    this->_hit = value;
}

uint8_t fb::game::equipment::master::damage() const
{
    return this->_damage;
}

void fb::game::equipment::master::damage(uint8_t value)
{
    this->_damage = value;
}

uint8_t fb::game::equipment::master::strength() const
{
    return this->_strength;
}

void fb::game::equipment::master::strength(uint8_t value)
{
    this->_strength = value;
}

uint8_t fb::game::equipment::master::intelligence() const
{
    return this->_intelligence;
}

void fb::game::equipment::master::intelligence(uint8_t value)
{
    this->_intelligence = value;
}

uint8_t fb::game::equipment::master::dexteritry() const
{
    return this->_dexteritry;
}

void fb::game::equipment::master::dexteritry(uint8_t value)
{
    this->_dexteritry = value;
}

uint32_t fb::game::equipment::master::base_hp() const
{
    return this->_base_hp;
}

void fb::game::equipment::master::base_hp(uint32_t value)
{
    this->_base_hp = value;
}

uint32_t fb::game::equipment::master::base_mp() const
{
    return this->_base_mp;
}

void fb::game::equipment::master::base_mp(uint32_t value)
{
    this->_base_mp = value;
}

float fb::game::equipment::master::hp_percentage() const
{
    return this->_hp_percentage;
}

void fb::game::equipment::master::hp_percentage(float value)
{
    this->_hp_percentage = value;
}

float fb::game::equipment::master::mp_percentage() const
{
    return this->_mp_percentage;
}

void fb::game::equipment::master::mp_percentage(float value)
{
    this->_mp_percentage = value;
}

uint8_t fb::game::equipment::master::healing_cycle() const
{
    return this->_healing_cycle;
}

void fb::game::equipment::master::healing_cycle(uint8_t value)
{
    this->_healing_cycle = value;
}

const std::string& fb::game::equipment::master::dress_script() const
{
    return this->_dress_script;
}

void fb::game::equipment::master::dress_script(const std::string& value)
{
    this->_dress_script = value;
}

const std::string& fb::game::equipment::master::undress_script() const
{
    return this->_undress_script;
}

void fb::game::equipment::master::undress_script(const std::string& value)
{
    this->_undress_script = value;
}

uint16_t fb::game::equipment::master::durability() const
{
    return this->_durability;
}

void fb::game::equipment::master::durability(uint16_t value)
{
    this->_durability = value;
}

bool fb::game::equipment::master::repair_enabled() const
{
    return this->_repair._enabled;
}

void fb::game::equipment::master::repair_enabled(bool value)
{
    this->_repair._enabled = value;
}

float fb::game::equipment::master::repair_price() const
{
    return this->_repair._price;
}

void fb::game::equipment::master::repair_price(float value)
{
    this->_repair._price = value;
}

bool fb::game::equipment::master::rename_enabled() const
{
    return this->_rename._enabled;
}

void fb::game::equipment::master::rename_enabled(bool value)
{
    this->_rename._enabled = value;
}

uint32_t fb::game::equipment::master::rename_price() const
{
    return this->_rename._price;
}

void fb::game::equipment::master::rename_price(uint32_t value)
{
    this->_rename._price = value;
}

fb::game::equipment::equipment(const equipment::master* master, listener* listener) : 
    item(master, listener)
{
    this->_durability = static_cast<const equipment::master*>(this->_master)->_durability;
}

fb::game::equipment::equipment(const equipment& right) :
    item(right)
{}

fb::game::equipment::~equipment()
{}


const std::string fb::game::equipment::name_trade() const
{
    std::stringstream sstream;
    float                   percentage = this->durability() / float(this->durability_base()) * 100;
    sstream << this->name() << '(' << std::fixed << std::setprecision(1) << percentage << "%)";

    return sstream.str();
}

bool fb::game::equipment::active()
{
    fb::game::item*         before = nullptr;
    auto                    slot = equipment::slot::UNKNOWN_SLOT;
    auto                    attr(fb::game::item::attrs(this->attr() & ~item::attrs::ITEM_ATTR_EQUIPMENT));
    switch(attr)
    {
    case item::attrs::ITEM_ATTR_WEAPON:
        before = this->_owner->items.weapon(static_cast<fb::game::weapon*>(this));
        slot = equipment::slot::WEAPON_SLOT;
        break;

    case item::attrs::ITEM_ATTR_ARMOR:
        before = this->_owner->items.armor(static_cast<fb::game::armor*>(this));
        slot = equipment::slot::ARMOR_SLOT;
        break;

    case item::attrs::ITEM_ATTR_SHIELD:
        before = this->_owner->items.shield(static_cast<fb::game::shield*>(this));
        slot = equipment::slot::SHIELD_SLOT;
        break;

    case item::attrs::ITEM_ATTR_HELMET:
        before = this->_owner->items.helmet(static_cast<fb::game::helmet*>(this));
        slot = equipment::slot::HELMET_SLOT;
        break;

    case item::attrs::ITEM_ATTR_RING:
        if(this->_owner->items.ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT) == nullptr)
        {
            slot = equipment::slot::LEFT_HAND_SLOT;
        }
        else
        {
            slot = equipment::slot::RIGHT_HAND_SLOT;
        }

        before = this->_owner->items.ring(static_cast<fb::game::ring*>(this));
        break;


    case item::attrs::ITEM_ATTR_AUXILIARY:
        if(this->_owner->items.auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT) == nullptr)
        {
            slot = equipment::slot::LEFT_AUX_SLOT;
        }
        else
        {
            slot = equipment::slot::RIGHT_AUX_SLOT;
        }

        before = this->_owner->items.auxiliary(static_cast<fb::game::auxiliary*>(this));
        break;

    default:
        throw equipment::not_equipment_exception();
    }

    // 인벤토리에서는 사라지지만 소유상태는 유지되므로
    // id를 유지시켜줘야 한다.
    auto id = this->_id;
    this->_owner->items.remove(*this);
    this->id(id);

    this->_owner->items.add(before);
    if(this->_listener != nullptr)
        this->_listener->on_equipment_on(*this->_owner, *this, slot);

    return true;
}

uint16_t fb::game::equipment::dress() const
{
    return static_cast<const master*>(this->_master)->_dress;
}

uint16_t fb::game::equipment::durability() const
{
    return this->_durability;
}

uint16_t fb::game::equipment::durability_base() const
{
    return static_cast<const master*>(this->_master)->_durability;
}

bool fb::game::equipment::repair_enabled() const
{
    return static_cast<const master*>(this->_master)->_repair._enabled;
}

float fb::game::equipment::repair_price() const
{
    return static_cast<const master*>(this->_master)->_repair._price;
}

bool fb::game::equipment::rename_enabled() const
{
    return static_cast<const master*>(this->_master)->_rename._enabled;
}

uint32_t fb::game::equipment::rename_price() const
{
    return static_cast<const master*>(this->_master)->_rename._price;
}

int16_t fb::game::equipment::defensive_physical() const
{
    return static_cast<const master*>(this->_master)->_defensive.physical;
}

int16_t fb::game::equipment::defensive_magical() const
{
    return static_cast<const master*>(this->_master)->_defensive.magical;
}

uint8_t fb::game::equipment::hit() const
{
    return static_cast<const master*>(this->_master)->_hit;
}

uint8_t fb::game::equipment::damage() const
{
    return static_cast<const master*>(this->_master)->_damage;
}

uint8_t fb::game::equipment::strength() const
{
    return static_cast<const master*>(this->_master)->_strength;
}

uint8_t fb::game::equipment::intelligence() const
{
    return static_cast<const master*>(this->_master)->_intelligence;
}

uint8_t fb::game::equipment::dexteritry() const
{
    return static_cast<const master*>(this->_master)->_dexteritry;
}

uint32_t fb::game::equipment::base_hp() const
{
    return static_cast<const master*>(this->_master)->_base_hp;
}

uint32_t fb::game::equipment::base_mp() const
{
    return static_cast<const master*>(this->_master)->_base_mp;
}

float fb::game::equipment::hp_percentage() const
{
    return static_cast<const master*>(this->_master)->_hp_percentage;
}

float fb::game::equipment::mp_percentage() const
{
    return static_cast<const master*>(this->_master)->_mp_percentage;
}

uint8_t fb::game::equipment::healing_cycle() const
{
    return static_cast<const master*>(this->_master)->_healing_cycle;
}

std::string fb::game::equipment::mid_message() const
{
    return std::string();
}

std::string fb::game::equipment::tip_message() const
{
    std::stringstream           sstream;

    sstream << this->name() << std::endl;
    sstream << "내구성: " << std::to_string(this->durability()) << '/' << std::to_string(this->durability_base()) << ' ' << std::fixed << std::setprecision(1) << (this->durability() / (float)this->durability_base()) * 100 << '%' << std::endl;
    sstream << this->mid_message();
    sstream << "무장:   " << std::to_string(this->defensive_physical()) << " Hit:  " << std::to_string(this->hit()) << " Dam:  " << std::to_string(this->damage());

    if(this->base_hp())
        sstream << std::left << std::setw(14) << std::endl << "체력치 상승:" << std::to_string(this->base_hp());

    if(this->base_mp())
        sstream << std::left << std::setw(14) << std::endl << "마력치 상승:" << std::to_string(this->base_mp());

    if(this->strength())
        sstream << std::left << std::setw(14) << std::endl << "힘 상승:" << std::to_string(this->strength());

    if(this->dexteritry())
        sstream << std::left << std::setw(14) << std::endl << "민첩성 상승:" << std::to_string(this->dexteritry());

    if(this->intelligence())
        sstream << std::left << std::setw(14) << std::endl << "지력 상승:" << std::to_string(this->intelligence());

    if(this->healing_cycle())
        sstream << std::left << std::setw(14) << std::endl << "재생력 상승:" << std::to_string(this->healing_cycle());

    std::stringstream class_stream;

    class_stream << std::endl << std::setw(14) << std::left;
    auto limit = this->limit();
    if(limit.cls == 0)
        class_stream << "직업제한무";
    else
        class_stream << fb::game::table::classes.class2name(limit.cls, 0) << "용";
    sstream << class_stream.str() << "레벨 " << std::to_string(limit.level) << " 이상";

    const std::string desc = this->desc();
    if(desc.empty() == false)
        sstream << std::endl << std::endl << desc;

    return sstream.str();
}

const std::string equipment::column(equipment::slot slot)
{
    switch(slot)
    {
    case fb::game::equipment::slot::WEAPON_SLOT:
        return "weapon";

    case fb::game::equipment::slot::ARMOR_SLOT:
        return "armor";

    case fb::game::equipment::slot::SHIELD_SLOT:
        return "shield";

    case fb::game::equipment::slot::HELMET_SLOT:
        return "helmet";

    case fb::game::equipment::slot::LEFT_HAND_SLOT:
        return "ring_left";

    case fb::game::equipment::slot::RIGHT_HAND_SLOT:
        return "ring_right";

    case fb::game::equipment::slot::LEFT_AUX_SLOT:
        return "aux_top";

    case fb::game::equipment::slot::RIGHT_AUX_SLOT:
        return "aux_bot";

    default:
        throw std::runtime_error("invalid equipment slot");
    }
}


//
// class weapon
//
fb::game::weapon::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color),
    _damage_range(range32_t(0, 0), range32_t(0, 0))
{}

fb::game::weapon::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color, const range32_t& small, const range32_t& large) : 
    fb::game::equipment::master(id, name, look, dress, color),
    _damage_range(small, large)
{
}

fb::game::weapon::master::~master()
{}

fb::game::item::attrs fb::game::weapon::master::attr() const
{
    return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_WEAPON);
}

fb::game::object* fb::game::weapon::master::make(listener* listener) const
{
    return new weapon(this, listener);
}

fb::game::weapon::types fb::game::weapon::master::weapon_type() const
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

const fb::game::range32_t& fb::game::weapon::master::damage_small() const
{
    return this->_damage_range.small;
}

void fb::game::weapon::master::damage_small(uint32_t min, uint32_t max)
{
    this->_damage_range.small.min = min;
    this->_damage_range.small.max = max;
}

const fb::game::range32_t& fb::game::weapon::master::damage_large() const
{
    return this->_damage_range.large;
}

void fb::game::weapon::master::damage_large(uint32_t min, uint32_t max)
{
    this->_damage_range.large.min = min;
    this->_damage_range.large.max = max;
}

uint16_t fb::game::weapon::master::sound() const
{
    return this->_sound;
}

void fb::game::weapon::master::sound(uint16_t value)
{
    this->_sound = value;
}

const std::string& fb::game::weapon::master::spell() const
{
    return this->_spell;
}

void fb::game::weapon::master::spell(const std::string& value)
{
    this->_spell = value;
}


fb::game::weapon::weapon(const master* master, listener* listener) : 
    equipment(master, listener)
{}

fb::game::weapon::weapon(const weapon& right) : 
    equipment(right)
{}

fb::game::weapon::~weapon()
{}


fb::game::weapon::types fb::game::weapon::weapon_type() const
{
    return static_cast<const master*>(this->_master)->weapon_type();
}

const fb::game::range32_t& fb::game::weapon::damage_small() const
{
    return static_cast<const master*>(this->_master)->_damage_range.small;
}

const fb::game::range32_t& fb::game::weapon::damage_large() const
{
    return static_cast<const master*>(this->_master)->_damage_range.large;
}

uint16_t fb::game::weapon::sound() const
{
    return static_cast<const master*>(this->_master)->_sound;
}

const std::string& fb::game::weapon::spell() const
{
    return static_cast<const master*>(this->_master)->_spell;
}

std::string fb::game::weapon::mid_message() const
{
    std::stringstream sstream;

    sstream << "파괴력: 　　 S:　" << std::to_string(this->damage_small().min) << 'm' << std::to_string(this->damage_small().max) << std::endl;
    sstream << "　　　  　 　L:　" << std::to_string(this->damage_large().min) << 'm' << std::to_string(this->damage_large().max) << std::endl;
    return sstream.str();
}


//
// armor::master methods
//
fb::game::armor::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::armor::master::~master()
{}

fb::game::item::attrs fb::game::armor::master::attr() const
{
    return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_ARMOR);
}

fb::game::object* fb::game::armor::master::make(listener* listener) const
{
    return new armor(this, listener);
}



//
// armor methods
//
fb::game::armor::armor(const master* master, listener* listener) : 
    equipment(master, listener)
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
// helmet::master methods
//
fb::game::helmet::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::helmet::master::~master()
{}

fb::game::item::attrs fb::game::helmet::master::attr() const
{
    return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_HELMET);
}

fb::game::object* fb::game::helmet::master::make(listener* listener) const
{
    return new helmet(this, listener);
}



//
// helmet methods
//
fb::game::helmet::helmet(const master* master, listener* listener) : 
    equipment(master, listener)
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
// shield::master methods
//
fb::game::shield::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::shield::master::~master()
{}

fb::game::item::attrs fb::game::shield::master::attr() const
{
    return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_SHIELD);
}

fb::game::object* fb::game::shield::master::make(listener* listener) const
{
    return new shield(this, listener);
}



//
// shield methods
//
fb::game::shield::shield(const master* master, listener* listener) : 
    equipment(master, listener)
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
// ring::master methods
//
fb::game::ring::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::ring::master::~master()
{}

fb::game::item::attrs fb::game::ring::master::attr() const
{
    return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_RING);
}

fb::game::object* fb::game::ring::master::make(listener* listener) const
{
    return new ring(this, listener);
}



//
// ring methods
//
fb::game::ring::ring(const master* master, listener* listener) : 
    equipment(master, listener)
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
// auxiliary::master methods
//
fb::game::auxiliary::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::auxiliary::master::~master()
{}

fb::game::item::attrs fb::game::auxiliary::master::attr() const
{
    return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT | item::attrs::ITEM_ATTR_AUXILIARY);
}

fb::game::object* fb::game::auxiliary::master::make(listener* listener) const
{
    return new auxiliary(this, listener);
}



//
// auxiliary methods
//
fb::game::auxiliary::auxiliary(const master* master, listener* listener) : 
    equipment(master, listener)
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
// arrow::master methods
//
fb::game::arrow::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::arrow::master::~master()
{}

fb::game::item::attrs fb::game::arrow::master::attr() const
{
    return item::attrs(item::attrs::ITEM_ATTR_EQUIPMENT);
}

fb::game::object* fb::game::arrow::master::make(listener* listener) const
{
    return new arrow(this, listener);
}



//
// arrow methods
//


fb::game::arrow::arrow(const master* master, listener* listener) : 
    equipment(master, listener)
{
}

fb::game::arrow::arrow(const arrow& right) : 
    equipment(right)
{
}

fb::game::arrow::~arrow()
{
}


bool fb::game::itemmix::contains(const item* item) const
{
    for(auto i : this->require)
    {
        if(item->based() == i.item && item->count() >= i.count)
            return true;
    }

    return false;
}

void fb::game::itemmix::require_add(fb::game::item::master* item, uint32_t count)
{
    this->require.push_back(element(item, count));
}

void fb::game::itemmix::success_add(fb::game::item::master* item, uint32_t count)
{
    this->success.push_back(element(item, count));
}

void fb::game::itemmix::failed_add(fb::game::item::master* item, uint32_t count)
{
    this->failed.push_back(element(item, count));
}

bool fb::game::itemmix::matched(const std::vector<item*>& items) const
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

fb::game::items::items(session& owner, listener* listener) :
    base_container(owner, item::MAX_SLOT),
    _owner(static_cast<fb::game::session&>(owner)),
    _listener(listener),
    _weapon(nullptr), _armor(nullptr), _helmet(nullptr), _shield(nullptr)
{
    memset(this->_rings, 0, sizeof(this->_rings));
    memset(this->_auxiliaries, 0, sizeof(this->_auxiliaries));
}

fb::game::items::~items()
{
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

uint8_t fb::game::items::equipment_off(fb::game::equipment::slot slot)
{
    try
    {
        if (this->free() == false)
            throw std::runtime_error(message::exception::INVENTORY_OVERFLOW);

        fb::game::item* item = nullptr;
        switch (slot)
        {
        case fb::game::equipment::slot::WEAPON_SLOT:
            item = this->_weapon;
            if (this->_weapon != nullptr)
                this->_weapon = nullptr;

            break;

        case fb::game::equipment::slot::ARMOR_SLOT:
            item = this->_armor;
            if (this->_armor != nullptr)
                this->_armor = nullptr;
            break;

        case fb::game::equipment::slot::SHIELD_SLOT:
            item = this->_shield;
            if (this->_shield != nullptr)
                this->_shield = nullptr;
            break;

        case fb::game::equipment::slot::HELMET_SLOT:
            item = this->_helmet;
            if (this->_helmet != nullptr)
                this->_helmet = nullptr;
            break;

        case fb::game::equipment::slot::LEFT_HAND_SLOT:
            item = this->_rings[0];
            if (this->_rings[0] != nullptr)
                this->_rings[0] = nullptr;
            break;

        case fb::game::equipment::slot::RIGHT_HAND_SLOT:
            item = this->_rings[1];
            if (this->_rings[1] != nullptr)
                this->_rings[1] = nullptr;
            break;

        case fb::game::equipment::slot::LEFT_AUX_SLOT:
            item = this->_auxiliaries[0];
            if (this->_auxiliaries[0] != nullptr)
                this->_auxiliaries[0] = nullptr;
            break;

        case fb::game::equipment::slot::RIGHT_AUX_SLOT:
            item = this->_auxiliaries[1];
            if (this->_auxiliaries[1] != nullptr)
                this->_auxiliaries[1] = nullptr;
            break;

        default:
            throw std::runtime_error("뭐지 병신 ㅋ");
        }

        auto index = this->add(item);
        if(this->_listener != nullptr)
        {
            this->_listener->on_updated(this->_owner, state_level::LEVEL_MAX);
            this->_listener->on_show(this->_owner, false);
            this->_listener->on_equipment_off(this->_owner, slot, index);
        }
        
        return index;
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(this->_owner, e.what());

        return 0xFF;
    }
}

uint8_t fb::game::items::add(fb::game::item& item)
{
    return this->add(&item);
}

uint8_t fb::game::items::add(fb::game::item* item)
{
    auto result = this->add(std::vector<fb::game::item*> { item });
    return result.empty() ? 
        0xFF : result[0];
}

std::vector<uint8_t> items::add(const std::vector<fb::game::item*>& items)
{
    std::vector<uint8_t>                indices;
    std::map<uint8_t, fb::game::item*>  gets, updates;

    for(auto item : items)
    {
        if(item == nullptr)
            continue;

        // 번들 형식의 아이템인 경우
        if(item->attr() & item::attrs::ITEM_ATTR_BUNDLE)
        {
            for(int i = 0; i < fb::game::item::MAX_SLOT; i++)
            {
                if(this->at(i) == nullptr)
                    continue;

                if(item->based() != this->at(i)->based())
                    continue;

                // 아이템을 합치고 남은 갯수로 설정한다.
                auto exists = this->at(i);
                auto remain = exists->fill(item->count());
                item->count(remain);

                if(this->_listener != nullptr)
                    this->_listener->on_item_update(static_cast<session&>(this->owner()), i);

                if(item->empty())
                    delete &item;

                updates.insert(std::make_pair(i, item));

                indices.push_back(i);
                break;
            }
        }

        // 그 이외의 아이템인 경우
        for(int i = 0; i < fb::game::item::MAX_SLOT; i++)
        {
            if(this->at(i) != nullptr)
                continue;

            item->_owner = &this->_owner;
            this->set(item, i);
            if(item->_listener != nullptr)
                item->_listener->on_item_update(static_cast<session&>(this->owner()), i);

            if(item->_map != nullptr)
                item->map(nullptr);

            if(this->_listener != nullptr)
            {
                if(item->id() == 0xFFFFFFFF)
                    gets.insert(std::make_pair(i, item));
                else
                    updates.insert(std::make_pair(i, item));
            }

            indices.push_back(i);
            break;
        }
    }

    if(this->_listener != nullptr)
    {
        this->_listener->on_item_get(this->_owner, gets);
        this->_listener->on_item_changed(this->_owner, updates);
    }

    return std::move(indices);
}

bool items::add(fb::game::item& item, uint8_t slot)
{
    if(this->at(slot) != nullptr)
        return false;

    this->set(&item, slot);
    item._owner = &this->_owner;
    if(item._listener != nullptr)
        item._listener->on_item_update(static_cast<session&>(this->owner()), slot);
    return true;
}

bool fb::game::items::reduce(uint8_t index, uint16_t count)
{
    auto                    item = this->at(index);
    if(item == nullptr)
        return false;

    this->at(index)->reduce(count);
    return true;
}

fb::game::item* fb::game::items::active(uint8_t index)
{
    try
    {
        this->_owner.state_assert(state::RIDING | state::GHOST);

        auto                    item = this->at(index);
        if(item == nullptr)
            return nullptr;

        item->active();
        if(item->empty())
        {
            delete item;
            return nullptr;
        }
        else
        {
            return item;
        }
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(this->_owner, e.what());

        return nullptr;
    }
}

uint8_t fb::game::items::inactive(equipment::slot slot)
{
    return this->equipment_off(slot);
}

uint8_t fb::game::items::to_index(const fb::game::item::master* item) const
{
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        auto now = this->at(i);

        if(now == nullptr)
            continue;

        if(now->based<item::master>() == item)
            return i;
    }

    return 0xFF;
}

uint8_t fb::game::items::to_index(const fb::game::item& item) const
{
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        auto now = this->at(i);
        if(now == &item)
            return i;
    }

    return 0xFF;
}

fb::game::equipment* items::wear(fb::game::equipment::slot slot, fb::game::equipment* item)
{
    switch(slot) // equipment::slot
    {
    case equipment::WEAPON_SLOT:
        return this->_owner.items.weapon(static_cast<fb::game::weapon*>(item));

    case equipment::ARMOR_SLOT:
        return this->_owner.items.armor(static_cast<fb::game::armor*>(item));

    case equipment::SHIELD_SLOT:
        return this->_owner.items.shield(static_cast<fb::game::shield*>(item));

    case equipment::HELMET_SLOT:
        return this->_owner.items.helmet(static_cast<fb::game::helmet*>(item));

    case equipment::LEFT_HAND_SLOT:
        return this->_owner.items.ring(static_cast<fb::game::ring*>(item), equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT);
        break;

    case equipment::RIGHT_HAND_SLOT:
        return this->_owner.items.ring(static_cast<fb::game::ring*>(item), equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT);

    case equipment::LEFT_AUX_SLOT:
        return this->_owner.items.auxiliary(static_cast<fb::game::auxiliary*>(item), equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT);

    case equipment::RIGHT_AUX_SLOT:
        return this->_owner.items.auxiliary(static_cast<fb::game::auxiliary*>(item), equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT);

    default:
        throw std::runtime_error("invalid equipment slot");
    }
}

fb::game::weapon* fb::game::items::weapon() const
{
    return this->_weapon;
}

fb::game::weapon* fb::game::items::weapon(fb::game::weapon* weapon)
{
    fb::game::weapon*       before = this->_weapon;

    this->_weapon = weapon;
    weapon->_owner = &this->_owner;
    
    if(this->_listener != nullptr)
        this->_listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::armor* fb::game::items::armor() const
{
    return this->_armor;
}

fb::game::armor* fb::game::items::armor(fb::game::armor* armor)
{
    fb::game::armor*        before = this->_armor;

    this->_armor = armor;
    armor->_owner = &this->_owner;

    if(this->_listener != nullptr)
        this->_listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::shield* fb::game::items::shield() const
{
    return this->_shield;
}

fb::game::shield* fb::game::items::shield(fb::game::shield* shield)
{
    fb::game::shield*       before = this->_shield;

    this->_shield = shield;
    shield->_owner = &this->_owner;

    if(this->_listener != nullptr)
        this->_listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::helmet* fb::game::items::helmet() const
{
    return this->_helmet;
}

fb::game::helmet* fb::game::items::helmet(fb::game::helmet* helmet)
{
    fb::game::helmet*       before = this->_helmet;

    this->_helmet = helmet;
    helmet->_owner = &this->_owner;

    if(this->_listener != nullptr)
        this->_listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::ring* fb::game::items::ring(equipment::EQUIPMENT_POSITION position) const
{
    return this->_rings[position];
}

fb::game::ring* fb::game::items::ring(fb::game::ring* ring)
{
    fb::game::ring*         before = nullptr;

    if(this->_rings[equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT] == nullptr)
    {
        before = this->ring(ring, equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT);
    }
    else
    {
        before = this->ring(ring, equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT);
    }
    ring->_owner = &this->_owner;

    if(this->_listener != nullptr)
        this->_listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::ring* fb::game::items::ring(fb::game::ring* ring, equipment::EQUIPMENT_POSITION position)
{
    auto before = this->_rings[position];
    this->_rings[position] = ring;

    if(this->_listener != nullptr)
        this->_listener->on_show(this->_owner, true);
    
    return before;
}


fb::game::auxiliary* fb::game::items::auxiliary(equipment::EQUIPMENT_POSITION position) const
{
    return this->_auxiliaries[position];
}

fb::game::auxiliary* fb::game::items::auxiliary(fb::game::auxiliary* auxiliary)
{
    fb::game::auxiliary*         before = nullptr;

    if(this->_auxiliaries[equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT] == nullptr)
    {
        before = this->auxiliary(auxiliary, equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT);
    }
    else
    {
        before = this->auxiliary(auxiliary, equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT);
    }
    auxiliary->_owner = &this->_owner;

    if(this->_listener != nullptr)
        this->_listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::auxiliary* items::auxiliary(fb::game::auxiliary* auxiliary, equipment::EQUIPMENT_POSITION position)
{
    auto before = this->_auxiliaries[position];
    this->_auxiliaries[position] = auxiliary;

    if(this->_listener != nullptr)
        this->_listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::item* fb::game::items::find(const std::string& name) const
{
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        auto item = this->at(i);
        if(item == nullptr)
            continue;

        if(item->name() == name)
            return item;
    }

    return nullptr;
}

fb::game::item* fb::game::items::drop(uint8_t slot, uint8_t count)
{
    auto&                   owner = static_cast<fb::game::session&>(this->owner());

    try
    {
        owner.state_assert(state::RIDING | state::GHOST);

        auto                    dropped = this->remove(slot, count, item::delete_attr::DELETE_DROP);
        if(dropped != nullptr)
        {
            dropped->map(owner.map(), owner.position());
            owner.action(action::PICKUP, duration::DURATION_PICKUP);
        }

        return dropped;
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(owner, e.what());
        return nullptr;
    }
}

void fb::game::items::pickup(bool boost)
{
    auto&                   owner = static_cast<fb::game::session&>(this->owner());

    try
    {
        auto                map = owner.map();
        if(map == nullptr)
            return;

        owner.action(action::PICKUP, duration::DURATION_PICKUP);


        std::string         message;

        // Pick up items in reverse order
        auto belows = map->belows(this->_owner.position(), fb::game::object::types::ITEM);
        for(int i = belows.size() - 1; i >= 0; i--)
        {
            auto            object = belows[i];
            auto            below = static_cast<fb::game::item*>(object);
            if(below->attr() & fb::game::item::attrs::ITEM_ATTR_CASH)
            {
                auto        cash = static_cast<fb::game::cash*>(below);
                auto        remain = owner.money_add(cash->chunk());
                cash->chunk(remain); // 먹고 남은 돈으로 설정

                if(this->_listener != nullptr)
                {
                    if(remain != 0)
                        this->_listener->on_notify(this->_owner, message::money::FULL);
                    this->_listener->on_updated(owner, state_level::LEVEL_MIN);
                }
            }
            else
            {
                auto            index = owner.items.add(below);
                if(index == -1)
                    break;
            }

            if(boost == false)
                break;
        }

        lua::thread ()
            .from("scripts/common/pickup.lua")
            .func("handle_pickup")
            .pushobject(owner)
            .resume(1);
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(owner, e.what());
    }
}

bool fb::game::items::throws(uint8_t slot)
{
    try
    {
        auto                    item = this->_owner.items.at(slot);
        if(item == nullptr)
            return false;

        if(item->unique())
            throw std::runtime_error(message::exception::CANNOT_THROW_ITEM);

        auto                    map = this->_owner.map();
        if(map == nullptr)
            throw std::exception();

        auto                    dropped = this->remove(slot, 1, item::delete_attr::DELETE_THROW);
        auto                    position = this->_owner.position();
        for(int i = 0; i < 7; i++)
        {
            auto                before = position;
            position.forward(this->_owner.direction());
            if(map->movable(position) == false)
            {
                position = before;
                break;
            }
        }

        if(this->_listener != nullptr)
            this->_listener->on_item_throws(this->_owner, *dropped, position);
        
        dropped->map(map, position);
        return true;
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(this->_owner, e.what());
        return false;
    }
}

fb::game::item* fb::game::items::remove(uint8_t slot, uint16_t count, item::delete_attr attr)
{
    auto& owner = static_cast<fb::game::session&>(this->owner());
    auto item = this->at(slot);
    if(item == nullptr)
        return nullptr;

    auto splitted = item->split(count);

    if(item == splitted)
    {
        base_container<fb::game::item>::remove(slot);
        
        if(this->_listener != nullptr)
            this->_listener->on_item_remove(owner, slot, attr);
    }

    if(this->_listener != nullptr)
    {
        this->_listener->on_item_update(owner, slot);

        if(this->at(slot) == nullptr)
            this->_listener->on_item_lost(this->_owner, std::vector<uint8_t> {slot});
        else
            this->_listener->on_item_changed(this->_owner, std::map<uint8_t, fb::game::item*> {{slot, this->at(slot)}});
    }

    return splitted;
}

fb::game::item* fb::game::items::remove(fb::game::item& item, uint16_t count, item::delete_attr attr)
{
    auto index = this->to_index(item);
    if(index == 0xFF)
        return nullptr;

    return this->remove(index, count, attr);
}

bool fb::game::items::swap(uint8_t src, uint8_t dest)
{
    if(fb::game::base_container<fb::game::item>::swap(src, dest) == false)
        return false;
    
    if(this->_listener != nullptr)
    {
        const auto              right = this->at(src);
        if(right != nullptr)
            this->_listener->on_item_update(this->_owner, src);
        else
            this->_listener->on_item_remove(this->_owner, src);
        
        const auto              left = this->at(dest);
        if(left != nullptr)
            this->_listener->on_item_update(this->_owner, dest);
        else
            this->_listener->on_item_remove(this->_owner, dest);

        this->_listener->on_item_swap(this->_owner, src, dest);
    }

    return true;
}

fb::game::itemmix::builder::builder(session& owner, listener* listener) : 
    _owner(owner),
    _listener(listener)
{
}

fb::game::itemmix::builder::~builder()
{
}

fb::game::itemmix::builder& fb::game::itemmix::builder::push(uint8_t index)
{
    auto item = this->_owner.items[index];
    if(item != nullptr)
        this->push_back(item);
    
    return *this;
}

bool fb::game::itemmix::builder::mix()
{
    try
    {
        auto                    itemmix = fb::game::table::mixes.find(*this);
        if(itemmix == nullptr)
            throw itemmix::no_match_exception();

        auto                    free_size = this->_owner.items.free_size();
        if(int(itemmix->success.size()) - int(itemmix->require.size()) > free_size)
            throw item::full_inven_exception();


        for(auto require : itemmix->require)
        {
            auto                index = this->_owner.items.to_index(require.item);
            if(index == 0xFF)
                return true;

            auto item = this->_owner.items.remove(index, require.count);
            if(item != nullptr)
                delete item;
        }

        auto                success = (std::rand() % 100) < itemmix->percentage;
        if(success)
        {
            for(auto success : itemmix->success)
            {
                auto        item = static_cast<fb::game::item*>(success.item->make(this->_listener));
                item->count(success.count);
                this->_owner.items.add(item);
            }

            if(this->_listener != nullptr)
                this->_listener->on_notify(this->_owner, fb::game::message::mix::SUCCESS);
        }
        else
        {
            for(auto failed : itemmix->failed)
            {
                auto        item = static_cast<fb::game::item*>(failed.item->make(this->_listener));
                item->count(failed.count);
                this->_owner.items.add(item);
            }

            if(this->_listener != nullptr)
                this->_listener->on_notify(this->_owner, fb::game::message::mix::FAILED);
        }

        return true;
    }
    catch(std::exception& e)
    { 
        if(this->_listener != nullptr)
            this->_listener->on_notify(this->_owner, e.what());
        return false;
    }
}
