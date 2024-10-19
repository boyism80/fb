#ifndef __TRADE_H__
#define __TRADE_H__

#include <vector>
#include <fb/stream.h>

namespace fb { namespace game {

class character;
class item;

class trade
{
public:
    enum class state : uint8_t;

public:
    interface listener;

private:
    character&                                _owner;
    character*                                _you      = nullptr;
    std::vector<fb::game::item*>            _items;
    fb::game::item*                         _selected = nullptr;
    uint32_t                                _money    = 0;
    bool                                    _locked   = false;

public:
    trade(character& owner);
    ~trade();

private:
    uint8_t                                 find(fb::game::item& item) const;
    uint8_t                                 add(fb::game::item& item);
    void                                    restore();
    void                                    flush();
    bool                                    flushable() const;
    void                                    end();

public:
    character*                                you() const;

    bool                                    begin(character& you);
    bool                                    trading() const;

    bool                                    up(fb::game::item& item);
    bool                                    up(uint8_t money);
    uint32_t                                money() const;

    bool                                    count(uint16_t count);

    bool                                    cancel();
    bool                                    lock();

    const std::vector<fb::game::item*>&     items() const;
    const fb::game::item*                   item(uint8_t index) const;
};

enum class trade::state : uint8_t
{
    REQUEST                                 = 0x00,
    UP_ITEM                                 = 0x01,
    ITEM_COUNT                              = 0x02,
    UP_MONEY                                = 0x03,
    CANCEL                                  = 0x04,
    LOCK                                    = 0x05,
};

interface trade::listener
{
    virtual void                            on_trade_begin(character& me, character& you) = 0;
    virtual void                            on_trade_bundle(character& me) = 0;
    virtual void                            on_trade_item(character& me, character& from, uint8_t index) = 0;
    virtual void                            on_trade_money(character& me, character& from) = 0;
    virtual void                            on_trade_cancel(character& me, character& from) = 0;
    virtual void                            on_trade_lock(character& me, bool mine) = 0;
    virtual void                            on_trade_failed(character& me) = 0;
    virtual void                            on_trade_success(character& me) = 0;
};

} }

#endif // !__TRADE_H__