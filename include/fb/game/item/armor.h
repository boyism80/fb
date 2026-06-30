#ifndef __ITEM_ARMOR_H__
#define __ITEM_ARMOR_H__

#include <fb/game/item/equipment.h>

namespace fb::game {

class armor : public equipment
{
public:
    armor(fb::game::server& server, const fb::model::armor& model, const initial_params& params = {});
    armor(const fb::game::armor& right);
    ~armor();
};

} // namespace fb::game

#endif // !__ITEM_ARMOR_H__
