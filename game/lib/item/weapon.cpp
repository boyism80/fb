#include <fb/game/item.h>

fb::game::weapon::master::master(const fb::game::weapon::master::config& config) : 
    fb::game::equipment::master(config),
    damage_range(config.small, config.large),
    sound(config.sound),
    spell(config.spell)
{ }


fb::game::weapon::master::~master()
{ }

fb::game::item::attrs fb::game::weapon::master::attr() const
{
    return item::attrs::WEAPON;
}

fb::game::weapon::types fb::game::weapon::master::weapon_type() const
{
    switch(this->look / 10000)
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


fb::game::weapon::weapon(fb::game::context& context, const master* master) : 
    equipment(context, master)
{ }

fb::game::weapon::weapon(const weapon& right) : 
    equipment(right)
{ }

fb::game::weapon::~weapon()
{ }

std::string fb::game::weapon::mid_message() const
{
    std::stringstream       sstream;
    auto                    master = this->based<fb::game::weapon>();
    
    sstream << "파괴력: 　　 S:　" << std::to_string(master->damage_range.small.min) << 'm' << std::to_string(master->damage_range.small.max) << std::endl;
    sstream << "　　　  　 　L:　" << std::to_string(master->damage_range.large.min) << 'm' << std::to_string(master->damage_range.large.max) << std::endl;
    return sstream.str();
}