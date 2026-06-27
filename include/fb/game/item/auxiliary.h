#ifndef __ITEM_AUXILIARY_H__
#define __ITEM_AUXILIARY_H__

#include <fb/game/item/equipment.h>

namespace fb::game {

class auxiliary : public equipment
{
public:
    auxiliary(fb::game::server& server, const fb::model::auxiliary& model, const initial_params& params = {});
    auxiliary(const fb::game::auxiliary& right);
    ~auxiliary();
};

} // namespace fb::game

#endif // !__ITEM_AUXILIARY_H__
