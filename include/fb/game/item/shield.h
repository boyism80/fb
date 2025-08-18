#ifndef __ITEM_SHIELD_H__
#define __ITEM_SHIELD_H__

#include <fb/game/item/equipment.h>

namespace fb::game {

class shield : public equipment
{
public:
    shield(fb::game::server& server, const fb::model::shield& model);
    shield(const shield& right);
    ~shield();
};

} // namespace fb::game

#endif // !__ITEM_SHIELD_H__
