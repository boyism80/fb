#ifndef __ITEM_CASH_H__
#define __ITEM_CASH_H__

#include <fb/game/item/base.h>

namespace fb::game {

class cash : public item
{
public:
    const uint32_t value = 0;

public:
    // clang-format off
    cash(fb::game::server& server, uint32_t chunk);
   ~cash();
    // clang-format on

private:
    static const fb::model::cash& match_model(fb::game::server& server, uint32_t value);

public:
    // clang-format off
    [[nodiscard]] async::task<std::shared_ptr<fb::game::cash>> replace(uint32_t value);
    [[nodiscard]] async::task<uint32_t>                        reduce(uint32_t value);
    // clang-format on

    // clang-format off
    bool        empty() const;
    std::string inven_name() const override;
    // clang-format on
};

} // namespace fb::game

#endif // !__ITEM_CASH_H__
