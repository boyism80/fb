#ifndef __ITEM_CASH_H__
#define __ITEM_CASH_H__

#include <fb/game/item/base.h>

namespace fb::game {

class cash : public item
{
public:
    const uint32_t value = 0;

public:
    cash(fb::game::server& server, uint32_t chunk);
    ~cash();

private:
    static const fb::model::cash& match_model(fb::game::server& server, uint32_t value);

public:
    async::task<std::shared_ptr<fb::game::cash>> replace(uint32_t value);
    async::task<uint32_t>                        reduce(uint32_t value);

    bool        empty() const;
    std::string inven_name() const override;
};

} // namespace fb::game

#endif // !__ITEM_CASH_H__
