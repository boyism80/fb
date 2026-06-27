#ifndef __ITEM_CONSUME_H__
#define __ITEM_CONSUME_H__

#include <fb/game/item/base.h>

namespace fb::game {

class consume : public item
{
public:
    consume(fb::game::server& server, const fb::model::consume& model, const initial_params& params = {});
    consume(const consume& right);
    ~consume();

public:
    bool active() override;
};

} // namespace fb::game

#endif // !__ITEM_CONSUME_H__
