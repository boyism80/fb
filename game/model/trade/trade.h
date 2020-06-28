#ifndef __TRADE_H__
#define __TRADE_H__

#include <vector>
#include "module/stream/stream.h"

namespace fb { namespace game {

class session;
class item;

class trade
{
private:
    session*                _owner;
    session*                _partner;
    std::vector<item*>      _items;
    item*                   _selected;
    uint32_t                _money;
    bool                    _locked;

public:
    trade(session& owner);
    ~trade();

private:
    uint8_t                 contains_core(item* item) const;

public:
    session*                partner() const;

    bool                    begin(session* partner);
    bool                    end();
    bool                    trading() const;

    item*                   selected();
    void                    select(item* item);

    uint8_t                 add(item* item);
    void                    money(uint32_t value);
    uint32_t                money() const;
    std::vector<uint8_t>    restore();
    void                    flush(session& session, std::vector<uint8_t>& indices);
    bool                    flushable(session& session) const;

    bool                    lock() const;
    void                    lock(bool value);

public:
    fb::ostream             make_show_stream(bool mine, uint8_t index) const;
    fb::ostream             make_money_stream(bool mine) const;
    fb::ostream             make_dialog_stream() const;
    fb::ostream             make_bundle_stream() const;
    fb::ostream             make_close_stream(const std::string& message) const;
    fb::ostream             make_lock_stream() const;
};

} }

#endif // !__TRADE_H__