#ifndef __ITEM_PACK_H__
#define __ITEM_PACK_H__

#include <fb/game/item/base.h>

namespace fb::game {

class pack : public item
{
private:
    uint32_t _durability = 0;

public:
    pack(fb::game::server& server, const fb::model::pack& model);
    pack(const pack& right);
    ~pack();

public:
    std::optional<uint32_t> durability() const;
    void                    durability(uint32_t value);
    std::string             inven_name() const override;

public:
    bool active() final;
};

} // namespace fb::game

#endif // !__ITEM_PACK_H__
