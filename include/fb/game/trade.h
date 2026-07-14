#ifndef __TRADE_H__
#define __TRADE_H__

#include <async/task.h>
#include <fb/stream.h>
#include <unordered_map>

namespace fb::game {

class character;
class item;

class trade
{
public:
    enum class state : uint8_t;

public:
    struct listener_t;

private:
    // Type aliases for internal use
    using character_ptr      = std::shared_ptr<character>;
    using character_weak_ptr = std::weak_ptr<character>;
    using item_ptr           = std::shared_ptr<fb::game::item>;
    using item_vector        = std::vector<item_ptr>;

    character_weak_ptr                   _owner;
    character_weak_ptr                   _you;
    std::unordered_map<uint8_t, uint8_t> _items;
    uint8_t                              _selected = 0xFF;
    uint32_t                             _money    = 0;
    bool                                 _locked   = false;

public:
    trade();
    ~trade();

private:
    uint8_t           add(uint8_t index);
    async::task<void> restore();
    item_ptr          find(const fb::model::item& item) const;
    void              assert_exchange(const fb::game::trade& trade) const;
    async::task<void> end();

    static async::task<void> exchange(trade& trade1, trade& trade2);

public:
    void owner(character_ptr owner);

    character_ptr     owner() const;
    character_ptr     you() const;
    async::task<bool> begin(character_ptr you);
    bool              trading() const;
    async::task<bool> up_item(uint8_t index);
    async::task<bool> up_money(uint32_t money);
    uint32_t          money() const;
    async::task<bool> count(uint16_t count);
    async::task<bool> cancel();
    async::task<bool> lock();
    const item_vector items() const;
    const item_ptr    item(uint8_t index) const;
};

enum class trade::state : uint8_t
{
    REQUEST    = 0x00,
    UP_ITEM    = 0x01,
    ITEM_COUNT = 0x02,
    UP_MONEY   = 0x03,
    CANCEL     = 0x04,
    LOCK       = 0x05,
};

struct trade::listener_t
{
    // clang-format off
    virtual async::task<void> on_trade_begin(character& me, character& you) = 0;
    virtual async::task<void> on_trade_bundle(character& me) = 0;
    virtual async::task<void> on_trade_item(character& me, character& you, uint8_t index, const fb::game::item& item) = 0;
    virtual async::task<void> on_trade_money(character& me, character& you, uint32_t money) = 0;
    virtual async::task<void> on_trade_cancel(character& me, character& you) = 0;
    virtual async::task<void> on_trade_lock(character& me, character& you) = 0;
    virtual async::task<void> on_trade_failed(character& me, character& you) = 0;
    virtual async::task<void> on_trade_success(character& me, character& you) = 0;
    // clang-format on
};

} // namespace fb::game

#endif // !__TRADE_H__