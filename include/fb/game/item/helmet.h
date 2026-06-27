#ifndef __ITEM_HELMET_H__
#define __ITEM_HELMET_H__

#include <fb/game/item/equipment.h>

namespace fb::game {

class helmet : public equipment
{
public:
    helmet(fb::game::server& server, const fb::model::helmet& model, const initial_params& params = {});
    helmet(const helmet& right);
    ~helmet();
};

} // namespace fb::game

#endif // !__ITEM_HELMET_H__
