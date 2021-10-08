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
    consume(const master* master, listener* listener, uint16_t count = 1);
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
    master(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0, uint16_t capacity = 1);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
    fb::game::item*                     make(fb::game::item::listener* listener) const;
};
#pragma endregion

} }

#endif // !__ITEM_CONSUME_H__
