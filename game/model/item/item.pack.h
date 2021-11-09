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
    uint16_t                            _durability = 0;
#pragma endregion

#pragma region constructor / destructor
public:
    pack(fb::game::context& context, const master* master);
    pack(const pack& right);
    ~pack();
#pragma endregion

#pragma region public method
public:
    std::optional<uint16_t>             durability() const;
    void                                durability(std::optional<uint16_t> value);
#pragma endregion

#pragma region override method
public:
    const std::string                   name_styled() const final;
    bool                                active() final;
#pragma endregion
};


#pragma region nested class
class pack::master : public fb::game::item::master
{
public:
    friend class pack;

public:
    uint16_t                            durability;

public:
    master(const std::string&                name, 
           uint16_t                          look, 
           uint8_t                           color,
           uint32_t                          id,
           uint32_t                          price,
           const fb::game::item::conditions& condition,
           penalties                         penalty,
           uint16_t                          capacity,
           const fb::game::item::trade&      trade,
           const fb::game::item::storage&    storage,
           std::string                       desc,
           std::string                       active_script,
           uint16_t                          durability);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
    virtual fb::game::item*             make(fb::game::context& context) const;
};
#pragma endregion

} }

#endif // !__ITEM_PACK_H__
