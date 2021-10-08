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
    enum types : uint8_t { NORMAL, SPEAR, BOW, FAN, UNKNOWN };
#pragma endregion

#pragma region constructor / destructor
public:
    weapon(const fb::game::weapon::master* master, listener* listener);
    weapon(const fb::game::weapon& right);
    ~weapon();
#pragma endregion

#pragma region public method
public:
    fb::game::weapon::types             weapon_type() const;
    const range32_t&                    damage_small() const;
    const range32_t&                    damage_large() const;
    uint16_t                            sound() const;
    const std::string&                  spell() const;
#pragma endregion

#pragma region override method
protected:
    std::string                         mid_message() const;
#pragma endregion
};


#pragma region nested class
struct weapon::damage_range
{
public:
    range32_t                           small, large;

public:
    damage_range(const range32_t& small, const range32_t& large) : small(small), large(large) {}
};

class weapon::master : public equipment::master
{
public:
    friend class weapon;

private:
    weapon::damage_range                _damage_range;
    uint16_t                            _sound;
    std::string                         _spell;

public:
    master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color);
    master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color, const range32_t& small, const range32_t& large);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
    virtual fb::game::item*             make(fb::game::item::listener* listener) const;
    fb::game::weapon::types             weapon_type() const;

    const range32_t&                    damage_small() const;
    void                                damage_small(uint32_t min, uint32_t max);
    const range32_t&                    damage_large() const;
    void                                damage_large(uint32_t min, uint32_t max);

    uint16_t                            sound() const;
    void                                sound(uint16_t value);

    const std::string&                  spell() const;
    void                                spell(const std::string& value);

};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_WEAPON_H__