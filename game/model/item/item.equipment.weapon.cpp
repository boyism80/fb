#include "model/item/item.equipment.weapon.h"

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
    return item::attrs::ITEM_ATTR_WEAPON;
}

fb::game::item* fb::game::weapon::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::weapon(this, dynamic_cast<fb::game::equipment::listener*>(listener));
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