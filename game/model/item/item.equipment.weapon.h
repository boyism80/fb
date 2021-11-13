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
    weapon(fb::game::context& context, const fb::game::weapon::master* master);
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
#pragma region structure
public:
    struct config : public fb::game::equipment::master::config
    {
    public:
        range32_t                       small; 
        range32_t                       large;
        uint16_t                        sound;
        std::string                     spell;
    };
#pragma endregion

public:
    friend class weapon;

public:
    const weapon::damage_range          damage_range;
    const uint16_t                      sound;
    const std::string                   spell;

public:
    master(const fb::game::weapon::master::config& config);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
    fb::game::weapon::types             weapon_type() const;

};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_WEAPON_H__