#ifndef __ITEM_EQUIPMENT_ARMOR_H__
#define __ITEM_EQUIPMENT_ARMOR_H__

#include "model/item/item.equipment.h"

namespace fb { namespace game {

class armor : public equipment
{
#pragma region forward nested declaration
public:
    class master;
#pragma endregion

#pragma region constructor / destructor
public:
    armor(fb::game::context& context, const fb::game::armor::master* master);
    armor(const fb::game::armor& right);
    ~armor();
#pragma endregion
};


#pragma region nested class
class armor::master : public equipment::master
{
public:
    master(const fb::game::equipment::master::config& config);
    ~master();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::armor(context, this);
    }
    virtual fb::game::item::attrs       attr() const;
};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_ARMOR_H__