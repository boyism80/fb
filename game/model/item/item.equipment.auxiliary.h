#ifndef __ITEM_EQUIPMENT_AUXILIARY_H__
#define __ITEM_EQUIPMENT_AUXILIARY_H__

#include "model/item/item.equipment.h"

namespace fb { namespace game {

class auxiliary : public equipment
{
#pragma region forward nested declaration
public:
    class master;
#pragma endregion

#pragma region constructor / destructor
public:
    auxiliary(const master* master, listener* listener);
    auxiliary(const auxiliary& right);
    ~auxiliary();
#pragma endregion
};


#pragma region nested class
class auxiliary::master : public equipment::master
{
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
           const fb::game::defensive&          defensive);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
    virtual fb::game::item*             make(fb::game::item::listener* listener) const;
};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_AUXILIARY_H__