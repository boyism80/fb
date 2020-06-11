#include "session.h"
#include "trade.h"
#include "db.h"

fb::game::trade::trade(session& owner) : 
    _owner(&owner),
    _selected(nullptr),
    _money(0),
    _locked(false),
    _partner(nullptr)
{
}

fb::game::trade::~trade()
{
}

uint8_t fb::game::trade::contains_core(fb::game::item* item) const
{
    for(int i = 0; i < this->_items.size(); i++)
    {
        if(this->_items[i]->based() == item->based())
            return i;
    }

    return 0xFF;
}

fb::game::session* fb::game::trade::partner() const
{
    return this->_partner;
}

bool fb::game::trade::begin(session* partner)
{
    if(this->_partner != nullptr)
        return false;

    this->_partner = partner;
    return true;
}

bool fb::game::trade::end()
{
    if(this->_partner == nullptr)
        return false;

    this->_partner = nullptr;
    return true;
}

bool fb::game::trade::trading() const
{
    return this->_partner != nullptr;
}

fb::game::item* fb::game::trade::selected()
{
    return this->_selected;
}

void fb::game::trade::select(fb::game::item* item)
{
    this->_selected = item;
}

uint8_t fb::game::trade::add(fb::game::item* item)
{
    if(item->attr() & item::attrs::ITEM_ATTR_BUNDLE)
    {
        uint8_t exists = this->contains_core(item);
        if(exists != 0xFF)
        {
            uint16_t count = this->_items[exists]->count() + item->count();
            this->_items[exists]->count(count);
            return exists;
        }
    }

    this->_items.push_back(item);
    return this->_items.size() - 1;
}

void fb::game::trade::money(uint32_t value)
{
    this->_money = value;
}

uint32_t fb::game::trade::money() const
{
    return this->_money;
}

std::vector<uint8_t> fb::game::trade::restore()
{
    std::vector<uint8_t> indices;
    for(auto item : this->_items)
    {
        for(int i = 0; i < item::MAX_SLOT; i++)
        {
            auto        own_item = this->_owner->items[i];
            if(own_item == nullptr)
                continue;

            if(!(own_item->attr() & item::attrs::ITEM_ATTR_BUNDLE))
                continue;

            if(own_item->based() != item->based())
                continue;

            own_item->count(own_item->count() + item->count());
            indices.push_back(i);

            delete item;
            item = nullptr;
            break;
        }

        if(item == nullptr)
            continue;

        auto            index = this->_owner->items.add(item);
        if(index != 0xFF)
            indices.push_back(index);
    }

    this->_items.clear();

    this->_owner->money_add(this->_money);
    this->_money = 0;

    this->_locked = false;
    return indices;
}

void fb::game::trade::flush(session& session, std::vector<uint8_t>& indices)
{
    indices.clear();

    for(auto item : this->_items)
    {
        indices.push_back(session.items.add(item));

        if(item->empty())
            delete item;
    }
    this->_items.clear();

    session.money_add(this->_money);
    this->_money = 0;

    this->_locked = false;
}

bool fb::game::trade::flushable(session& session) const
{
    uint32_t before_money = session.money();
    uint32_t remain = session.money_add(this->_money);
    session.money(before_money);

    if(remain != 0)
        return false;

    uint8_t free_size = session.items.free_size();
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        fb::game::item* item = session.items[i];
        if(item == nullptr)
            continue;

        if(!(item->attr() & item::attrs::ITEM_ATTR_BUNDLE))
            continue;

        uint8_t contains_core = this->contains_core(item);
        if(contains_core == 0xFF)
            continue;

        if(item->free_space() < this->_items[contains_core]->count())
            return false;

        free_size++;
    }

    if(free_size < this->_items.size())
        return false;

    return true;
}

bool fb::game::trade::lock() const
{
    return this->_locked;
}

void fb::game::trade::lock(bool value)
{
    this->_locked = value;
}

fb::ostream fb::game::trade::make_show_stream(bool mine, uint8_t index) const
{
    try
    {
        fb::ostream             ostream;
        const auto              item = this->_items[index];

        ostream.write_u8(0x42)
            .write_u8(0x02)
            .write_u8(mine ? 0x00 : 0x01)
            .write_u8(index) // trade slot index
            .write_u16(item->look())
            .write_u8(item->color())
            .write(item->name_trade())
            .write_u8(0x00);

        return ostream;
    }
    catch(std::exception& e)
    {
        return fb::ostream();
    }
}

fb::ostream fb::game::trade::make_money_stream(bool mine) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x42)
        .write_u8(0x03)
        .write_u8(mine ? 0x00 : 0x01)
        .write_u32(this->_money)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::trade::make_dialog_stream() const
{
    fb::ostream             ostream;
    const auto              class_name = db::class2name(this->_owner->cls(), this->_owner->promotion());
    if(class_name == nullptr)
        return ostream;

    std::stringstream sstream;
    sstream << this->_owner->name() << '(' << class_name->c_str() << ')';

    ostream.write_u8(0x42)
        .write_u8(0x00)
        .write_u32(this->_owner->id())
        .write(sstream.str())
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::trade::make_bundle_stream() const
{
    fb::ostream             ostream;

    ostream.write_u8(0x42)
        .write_u8(0x01)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::trade::make_close_stream(const std::string& message) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x42)
        .write_u8(0x04)
        .write(message, true)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::trade::make_lock_stream() const
{
    fb::ostream             ostream;

    ostream.write_u8(0x42)
        .write_u8(0x05)
        .write_u8(0x00);

    return ostream;
}