#ifndef __ITEM_RING_H__
#define __ITEM_RING_H__

#include <fb/game/item/equipment.h>

namespace fb::game {

class ring : public equipment
{
public:
    ring(fb::game::server& server, const fb::model::ring& model);
    ring(const ring& right);
    ~ring();
};

} // namespace fb::game

#endif // !__ITEM_RING_H__
