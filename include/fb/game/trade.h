#ifndef __TRADE_H__
#define __TRADE_H__

#include <async/task.h>
#include <fb/stream.h>

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace fb::game {

class character;
class item;

// One session is shared by both characters; each character holds it in character::trade while trading.
class trade : public std::enable_shared_from_this<trade>
{
public:
    enum class state : uint8_t;

public:
    struct listener_t;

private:
    using item_ptr = std::shared_ptr<fb::game::item>;

    struct side
    {
        std::weak_ptr<character>             owner;
        std::unordered_map<uint8_t, uint8_t> items;
        uint8_t                              selected = 0xFF;
        uint64_t                             money    = 0;
        bool                                 locked   = false;
    };

    class exchange_error : public std::runtime_error
    {
    public:
        const size_t side;

    public:
        exchange_error(size_t side, const std::string& message) :
            std::runtime_error(message),
            side(side)
        { }
    };

    std::array<side, 2> _sides;

public:
    trade(character& me, character& you);
    ~trade();

private:
    size_t            index(const character& ch) const;
    bool              locked() const;
    uint8_t           add(side& side, uint8_t index);
    void              restore();
    void              end();
    void              assert_exchange(size_t i) const;
    async::task<void> exchange();

public:
    static bool                     begin(character& me, character& you);
    bool                            up_item(character& ch, uint8_t index);
    bool                            up_money(character& ch, uint64_t money);
    uint64_t                        money(const character& ch) const;
    bool                            count(character& ch, uint16_t count);
    bool                            cancel(character& ch);
    [[nodiscard]] async::task<bool> lock(character& ch);
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
    virtual void on_trade_begin(character& me, character& you)                                                                 = 0;
    virtual void on_trade_bundle(character& me)                                                                                = 0;
    virtual void on_trade_item(character& me, character& you, uint8_t index, const fb::game::item& item)                       = 0;
    virtual void on_trade_money(character& me, character& you, uint64_t money)                                                 = 0;
    virtual void on_trade_cancel(character& me, character& you)                                                                = 0;
    virtual void on_trade_lock(character& me, character& you)                                                                  = 0;
    virtual void on_trade_failed(character& me, character& you, const std::string& me_message, const std::string& you_message) = 0;
    virtual void on_trade_success(character& me, character& you)                                                               = 0;
    // clang-format on
};

} // namespace fb::game

#endif // !__TRADE_H__
