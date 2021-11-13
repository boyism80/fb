#ifndef __ITEM_EQUIPMENT_SHIELD_H__
#define __ITEM_EQUIPMENT_SHIELD_H__

#include "model/item/item.equipment.h"

namespace fb { namespace game {
    
class shield : public equipment
{
#pragma region forward nested declaration
public:
    class master;
#pragma endregion

#pragma region constructor / destructor
public:
    shield(fb::game::context& context, const master* master);
    shield(const shield& right);
    ~shield();
#pragma endregion
};


#pragma region nested class
class shield::master : public equipment::master
{
public:
    master(const fb::game::equipment::master::config& config);
    ~master();
    
public:
    virtual fb::game::item::attrs       attr() const;
};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_SHIELD_H__