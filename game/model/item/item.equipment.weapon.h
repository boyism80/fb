#ifndef __ITEM_EQUIPMENT_WEAPON_H__
#define __ITEM_EQUIPMENT_WEAPON_H__

#include "model/item/item.equipment.h"

namespace fb { namespace game {
    
class weapon : public equipment
{
#pragma region forward nested declaration
public:
    struct damage_range;

public:
    class master;
#pragma endregion

#pragma region enum
public:
    enum class types : uint8_t { NORMAL, SPEAR, BOW, FAN, UNKNOWN };
#pragma endregion

#pragma region constructor / destructor
public:
    weapon(const fb::game::weapon::master* master, listener* listener);
    weapon(const fb::game::weapon& right);
    ~weapon();
#pragma endregion

#pragma region override method
protected:
    std::string                         mid_message() const final;
#pragma endregion
};


#pragma region nested class
struct weapon::damage_range
{
public:
    range32_t                           small, large;

public:
    damage_range(const range32_t& small, const range32_t& large) : small(small), large(large) { }
};

class weapon::master : public equipment::master
{
public:
    friend class weapon;

public:
    const weapon::damage_range          damage_range;
    const uint16_t                      sound;
    const std::string                   spell;

public:
    master(const std::string&                  name, 
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
           const std::string&                  spell);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
    virtual fb::game::item*             make(fb::game::item::listener* listener) const;
    fb::game::weapon::types             weapon_type() const;

};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_WEAPON_H__