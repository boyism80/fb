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
    auxiliary(fb::game::context& context, const master* master);
    auxiliary(const auxiliary& right);
    ~auxiliary();
#pragma endregion
};


#pragma region nested class
class auxiliary::master : public equipment::master
{
public:
    master(const fb::game::equipment::master::config& config);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_AUXILIARY_H__