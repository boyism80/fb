#include "model/item/item.equipment.h"
#include "model/acceptor/acceptor.game.h"

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

fb::game::item* fb::game::equipment::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::equipment(this, dynamic_cast<fb::game::equipment::listener*>(listener));
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

std::optional<uint16_t> fb::game::equipment::master::durability() const
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
    float                   percentage = this->_durability / float(this->durability_base()) * 100;
    sstream << this->name() << '(' << std::fixed << std::setprecision(1) << percentage << "%)";

    return sstream.str();
}

bool fb::game::equipment::active()
{
    fb::game::item*         before = nullptr;
    auto                    slot = equipment::slot::UNKNOWN_SLOT;
    switch(this->attr())
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
        if(this->_owner->items.ring(equipment::position::EQUIPMENT_LEFT) == nullptr)
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
        if(this->_owner->items.auxiliary(equipment::position::EQUIPMENT_LEFT) == nullptr)
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
    this->_owner->items.remove(*this);

    this->_owner->items.add(before);

    auto listener = this->_owner->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_equipment_on(*this->_owner, *this, slot);

    return true;
}

uint16_t fb::game::equipment::dress() const
{
    return static_cast<const master*>(this->_master)->_dress;
}

std::optional<uint16_t> fb::game::equipment::durability() const
{
    return this->_durability;
}

void fb::game::equipment::durability(std::optional<uint16_t> value)
{
    this->_durability = value.value_or(0);
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
    sstream << "내구성: " << std::to_string(this->_durability) << '/' << std::to_string(this->durability_base()) << ' ' << std::fixed << std::setprecision(1) << (this->_durability / (float)this->durability_base()) * 100 << '%' << std::endl;
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
    auto condition = this->condition();
    if(condition.cls == 0)
        class_stream << "직업제한무";
    else
        class_stream << fb::game::table::classes.class2name(condition.cls, 0) << "용";
    sstream << class_stream.str() << "레벨 " << std::to_string(condition.level) << " 이상";

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