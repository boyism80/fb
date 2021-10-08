#ifndef __ITEM_PACK_H__
#define __ITEM_PACK_H__

#include "model/item/item.base.h"

namespace fb { namespace game {

class pack : public item
{
#pragma region forward nested declaration
public:
    class master;
#pragma endregion

#pragma region private field
private:
    uint16_t                            _durability;
#pragma endregion

#pragma region constructor / destructor
public:
    pack(const master* master, listener* listener);
    pack(const pack& right);
    ~pack();
#pragma endregion

#pragma region public method
public:
    std::optional<uint16_t>             durability() const;
    void                                durability(std::optional<uint16_t> value);
    uint16_t                            base_durability() const;
#pragma endregion

#pragma region override method
public:
    const std::string                   name_styled() const;
    bool                                active();
#pragma endregion
};


#pragma region nested class
class pack::master : public fb::game::item::master
{
public:
    friend class pack;

private:
    uint16_t                            _durability;

public:
    master(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0, uint16_t durability = 200);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
    virtual fb::game::item*             make(fb::game::item::listener* listener) const;
};
#pragma endregion

} }

#endif // !__ITEM_PACK_H__
