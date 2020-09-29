#ifndef __TRADE_H__
#define __TRADE_H__

#include <vector>
#include "module/stream/stream.h"

namespace fb { namespace game {

class session;
class item;
class listener;

class trade
{
#pragma region enum
public:
    enum state : uint8_t
    {
        REQUEST                             = 0x00,
        UP_ITEM                             = 0x01,
        ITEM_COUNT                          = 0x02,
        UP_MONEY                            = 0x03,
        CANCEL                              = 0x04,
        LOCK                                = 0x05,
    };
#pragma endregion

private:
    listener*                               _listener;
    session&                                _owner;
    session*                                _you;
    std::vector<fb::game::item*>            _items;
    fb::game::item*                         _selected;
    uint32_t                                _money;
    bool                                    _locked;

public:
    trade(session& owner, listener* listener);
    ~trade();

private:
    uint8_t                                 find(fb::game::item& item) const;
    uint8_t                                 add(fb::game::item& item);
    void                                    restore();
    void                                    flush();
    bool                                    flushable() const;
    void                                    end();

public:
    session*                                you() const;

    bool                                    begin(session& you);
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

} }

#endif // !__TRADE_H__