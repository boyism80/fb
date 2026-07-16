#ifndef __ITEM_PACK_H__
#define __ITEM_PACK_H__

#include <fb/game/item/base.h>

namespace fb::game {

class pack : public item
{
private:
    uint32_t _durability = 0;

public:
    // clang-format off
    pack(fb::game::server& server, const fb::model::pack& model, const initial_params& params = {});
    pack(const pack& right);
    ~pack();
    // clang-format on

public:
    // clang-format off
    std::optional<uint32_t> durability() const;
    void                    durability(uint32_t value);
    std::string             inven_name() const override;
    // clang-format on

public:
    [[nodiscard]] async::task<bool> active() override final;
};

} // namespace fb::game

#endif // !__ITEM_PACK_H__
