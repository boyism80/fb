#ifndef __ITEM_CONSUME_H__
#define __ITEM_CONSUME_H__

#include "model/item/item.base.h"

namespace fb { namespace game {

class consume : public item
{
#pragma region forward nested declaration
public:
    class master;
#pragma endregion

#pragma region constructor / destructor
public:
    consume(fb::game::context& context, const master* master, uint16_t count = 1);
    consume(const consume& right);
    ~consume();
#pragma endregion

#pragma region public method
public:
    bool                                active();
#pragma endregion
};


#pragma region nested class
class consume::master : public fb::game::item::master
{
public:
    friend class consume;

public:
    master(const fb::game::item::master::config& config);
    ~master();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::consume(context, this, count);
    }
    virtual fb::game::item::attrs       attr() const;
};
#pragma endregion

} }

#endif // !__ITEM_CONSUME_H__
