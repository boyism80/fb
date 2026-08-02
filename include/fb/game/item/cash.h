#ifndef __ITEM_CASH_H__
#define __ITEM_CASH_H__

#include <fb/game/item/base.h>

namespace fb::game {

class cash : public item
{
public:
    const uint64_t value = 0;

public:
    // clang-format off
    cash(fb::game::server& server, uint64_t chunk);
   ~cash();
    // clang-format on

public:
    const fb::model::cash& model() const override;

private:
    static const fb::model::cash& match_model(fb::game::server& server, uint64_t value);

public:
    // clang-format off
    [[nodiscard]] async::task<std::shared_ptr<fb::game::cash>> replace(uint64_t value);
    [[nodiscard]] async::task<uint64_t>                        reduce(uint64_t value);
    // clang-format on

    // clang-format off
    bool        empty() const;
    std::string inven_name() const override;
    // clang-format on
};

} // namespace fb::game

#endif // !__ITEM_CASH_H__
