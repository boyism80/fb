#ifndef __ITEM_CASH_H__
#define __ITEM_CASH_H__

#include "model/item/item.base.h"

namespace fb { namespace game {

class cash : public item
{
#pragma region forward nested declaration
public:
    class master;
#pragma endregion

#pragma region static const value
public:
    static const master                 BRONZE, BRONZE_BUNDLE, SILVER, SILVER_BUNDLE, GOLD, GOLD_BUNDLE;
#pragma endregion

#pragma region private field
private:
    uint32_t                            _chunk = 0;
#pragma endregion

#pragma region constructor / destructor
public:
    cash(fb::game::context& context, uint32_t chunk);
    ~cash();
#pragma endregion

#pragma region virtual method
public:
    virtual const std::string           name_styled() const;
#pragma endregion

#pragma region public method
public:
    uint32_t                            chunk() const;
    fb::game::cash*                     chunk(uint32_t value);
    uint32_t                            chunk_reduce(uint32_t value);

    bool                                empty() const;
#pragma endregion
};


#pragma region nested class
class cash::master : public fb::game::item::master
{
public:
    master(const fb::game::item::master::config& config);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
};
#pragma endregion

} }

#endif // !__ITEM_CASH_H__
