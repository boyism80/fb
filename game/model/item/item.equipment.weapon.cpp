#include "model/item/item.equipment.weapon.h"

fb::game::weapon::master::master(const std::string&                  name, 
                                 uint16_t                            look, 
                                 uint8_t                             color,
                                 uint32_t                            id,
                                 uint32_t                            price,
                                 const fb::game::item::conditions&   condition,
                                 penalties                           penalty,
                                 uint16_t                            capacity,
                                 const fb::game::item::trade&        trade,
                                 const fb::game::item::storage&      storage,
                                 std::string                         desc,
                                 std::string                         active_script,
                                 uint16_t                            dress,
                                 uint16_t                            durability,
                                 const fb::game::equipment::repair&  repair,
                                 const fb::game::equipment::rename&  rename,
                                 const std::string&                  dress_script,
                                 const std::string&                  undress_script,
                                 uint8_t                             hit,
                                 uint8_t                             damage,
                                 uint8_t                             strength,
                                 uint8_t                             intelligence,
                                 uint8_t                             dexteritry,
                                 uint32_t                            base_hp,
                                 uint32_t                            base_mp,
                                 float                               hp_percentage,
                                 float                               mp_percentage,
                                 uint8_t                             healing_cycle,
                                 const fb::game::defensive&          defensive,
                                 const range32_t&                    small, 
                                 const range32_t&                    large,
                                 uint16_t                            sound,
                                 const std::string&                  spell) : fb::game::equipment::master(name,
                                                                                                          look,
                                                                                                          color,
                                                                                                          id,
                                                                                                          price,
                                                                                                          condition,
                                                                                                          penalty,
                                                                                                          capacity,
                                                                                                          trade,
                                                                                                          storage,
                                                                                                          desc,
                                                                                                          active_script,
                                                                                                          dress,
                                                                                                          durability,
                                                                                                          repair,
                                                                                                          rename,
                                                                                                          dress_script,
                                                                                                          undress_script,
                                                                                                          hit,
                                                                                                          damage,
                                                                                                          strength,
                                                                                                          intelligence,
                                                                                                          dexteritry,
                                                                                                          base_hp,
                                                                                                          base_mp,
                                                                                                          hp_percentage,
                                                                                                          mp_percentage,
                                                                                                          healing_cycle,
                                                                                                          defensive),
                                                                              damage_range(small, large),
                                                                              sound(sound),
                                                                              spell(spell)
{ }


fb::game::weapon::master::~master()
{ }

fb::game::item::attrs fb::game::weapon::master::attr() const
{
    return item::attrs::WEAPON;
}

fb::game::item* fb::game::weapon::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::weapon(this, dynamic_cast<fb::game::equipment::listener*>(listener));
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


fb::game::weapon::weapon(const master* master, listener* listener) : 
    equipment(master, listener)
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