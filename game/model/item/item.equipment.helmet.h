#ifndef __ITEM_EQUIPMENT_HELMET_H__
#define __ITEM_EQUIPMENT_HELMET_H__

#include "model/item/item.equipment.h"

namespace fb { namespace game {
    
class helmet : public equipment
{
#pragma region forward nested declaration
public:
    class master;
#pragma endregion

#pragma region constructor / destructor
public:
    helmet(fb::game::context& context, const master* master);
    helmet(const helmet& right);
    ~helmet();
#pragma endregion
};


#pragma region nested class
class helmet::master : public equipment::master
{
public:
    master(const fb::game::equipment::master::config& config);
    ~master();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::helmet(context, this);
    }
    virtual fb::game::item::attrs       attr() const;
};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_HELMET_H__