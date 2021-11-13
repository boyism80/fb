#ifndef __ITEM_EQUIPMENT_ARROW_H__
#define __ITEM_EQUIPMENT_ARROW_H__

#include "model/item/item.equipment.h"

namespace fb { namespace game {

class bow : public equipment
{
#pragma region forward nested declaration
public:
    class master;
#pragma endregion

#pragma region constructor / destructor
public:
    bow(fb::game::context& context, const master* master);
    bow(const bow& right);
    ~bow();
#pragma endregion
};


#pragma region nested class
class bow::master : public equipment::master
{
public:
    master(const fb::game::equipment::master::config& config);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_ARROW_H__