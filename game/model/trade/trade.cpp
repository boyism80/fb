#include <model/session/session.h>
#include <model/trade/trade.h>
#include <model/master/master.h>
#include <model/listener/listener.h>

fb::game::trade::trade(session& owner, listener* listener) : 
    _owner(owner),
    _listener(listener),
    _selected(nullptr),
    _money(0),
    _locked(false),
    _you(nullptr)
{
}

fb::game::trade::~trade()
{
}

uint8_t fb::game::trade::find(fb::game::item& item) const
{
    for(int i = 0; i < this->_items.size(); i++)
    {
        if(this->_items[i]->based() == item.based())
            return i;
    }

    return 0xFF;
}

fb::game::session* fb::game::trade::you() const
{
    return this->_you;
}

bool fb::game::trade::begin(session& you)
{
    try
    {
        if(this->_owner.id() == you.id())
        {
            // 자기 자신과 거래를 하려고 시도하는 경우
            return false;
        }

        if(this->_owner.option(options::TRADE) == false)
        {
            throw std::runtime_error(message::trade::REFUSED_BY_ME);
        }

        if(you.option(options::TRADE) == false)
        {
            // 상대방이 교환 거부중
            std::stringstream sstream;
            sstream << you.name() << message::trade::REFUSED_BY_PARTNER;
            throw std::runtime_error(sstream.str());
        }

        if(this->trading())
        {
            return false;
        }

        if(you.trade.trading())
        {
            // 상대방이 이미 교환중
            std::stringstream sstream;
            sstream << you.name() << message::trade::PARTNER_ALREADY_TRADING;
            throw std::runtime_error(sstream.str());
        }

        if(this->_owner.sight(you) == false)
        {
            // 상대방이 시야에서 보이지 않음
            throw std::runtime_error(message::trade::PARTNER_INVISIBLE);
        }

        if(this->_owner.distance_sqrt(you) > 16)
        {
            // 상대방과의 거리가 너무 멈
            std::stringstream sstream;
            sstream << you.name() << message::trade::PARTNER_TOO_FAR;
            
            throw std::runtime_error(sstream.str());
        }

        this->_you = &you;
        you.trade._you = &this->_owner;

        if(this->_listener != nullptr)
        {
            this->_listener->on_trade_begin(this->_owner, you);
            this->_listener->on_trade_begin(you, this->_owner);
        }

        return true;
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(this->_owner, e.what());

        return false;
    }
}

void fb::game::trade::end()
{
    if(this->_you != nullptr)
    {
        this->_you->trade._you = nullptr;
        this->_you->trade._locked = false;
    }

    this->_you = nullptr;
    this->_locked = false;
}

bool fb::game::trade::trading() const
{
    return this->_you != nullptr;
}

bool fb::game::trade::up(item& item)
{
    try
    {
        if(this->trading() == false)
            throw std::runtime_error(message::trade::NOT_TRADING);

        if(item.unique())
            throw std::runtime_error(message::trade::NOT_ALLOWED_TO_TRADE);

        if((item.attr() & fb::game::item::attrs::ITEM_ATTR_BUNDLE) && (item.count() > 1))
        {
            // 묶음 단위의 아이템 형식 거래 시도
            this->_selected = &item;
            if (this->_listener != nullptr)
                this->_listener->on_trade_bundle(this->_owner);
        }
        else
        {
            // 일반 아이템의 거래 시도
            auto splitted = this->_owner.items.remove(item);
            auto index = this->add(*splitted);
            if(index == 0xFF)
                return false;

            if (this->_listener != nullptr)
            {
                this->_listener->on_trade_item(this->_owner, this->_owner, index);
                this->_listener->on_trade_item(*this->_you, this->_owner, index);
            }
        }

        return true;
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(this->_owner, e.what(), message::type::POPUP);
        
        return false;
    }
}

bool fb::game::trade::up(uint8_t money)
{
    try
    {
        // 입력한 금전 양을 계속해서 빼면 안된다.
        // 100전 입력한 경우 -1, -10, -100 이렇게 까여버림
        auto                total = this->_owner.money() + this->_money;
        if(money > total)
            money = total;

        this->_owner.money(total - money);
        this->_money = money;

        if(this->_listener != nullptr)
        {
            this->_listener->on_trade_money(this->_owner, this->_owner);
            this->_listener->on_trade_money(this->_owner, this->_owner);
        }
        
        return true;
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(this->_owner, e.what(), message::type::POPUP);

        return false;
    }
}

bool fb::game::trade::count(uint16_t count)
{
    try
    {
        if(this->trading() == false)
            throw std::runtime_error(message::trade::NOT_TRADING);

        if(this->_selected == nullptr)
            throw std::runtime_error(message::trade::NOT_SELECTED);

        if(this->_selected->unique())
            throw std::runtime_error(message::trade::NOT_ALLOWED_TO_TRADE);

        if(this->_selected->count() < count)
            throw std::runtime_error(message::trade::INVALID_COUNT);

        auto splitted = this->_owner.items.remove(*this->_selected, count);
        auto index = this->add(*splitted);

        if(this->_listener != nullptr)
        {
            this->_listener->on_trade_item(this->_owner, this->_owner, index);
            this->_listener->on_trade_item(*this->_you, this->_owner, index);
        }

        this->_selected = nullptr;
        return true;
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(this->_owner, e.what(), message::type::POPUP);

        return false;
    }
}

bool fb::game::trade::cancel()
{
    try
    {
        if(this->trading() == false)
            throw std::runtime_error(message::trade::NOT_TRADING);

        this->restore();
        this->_you->trade.restore();

        if(this->_listener != nullptr)
        {
            this->_listener->on_trade_cancel(this->_owner);
            this->_listener->on_trade_cancel(*this->_you);
        }

        this->end();
        return true;
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(this->_owner, e.what(), message::type::POPUP);

        return false;
    }
}

uint8_t fb::game::trade::add(fb::game::item& item)
{
    if(item.attr() & item::attrs::ITEM_ATTR_BUNDLE)
    {
        auto exists = this->find(item);
        if(exists != 0xFF)
        {
            auto count = this->_items[exists]->count() + item.count();
            this->_items[exists]->count(count);
            return exists;
        }
    }

    this->_items.push_back(&item);
    return uint8_t(this->_items.size() - 1);
}

void fb::game::trade::restore()
{
    for(auto item : this->_items)
        this->_owner.items.add(item);

    this->_items.clear();

    this->_owner.money_add(this->_money);
    this->_money = 0;
}

void fb::game::trade::flush()
{
    for(auto item : this->_items)
        this->_you->items.add(item);
    this->_items.clear();

    this->_you->money_add(this->_money);
    this->_money = 0;
}

bool fb::game::trade::flushable() const
{
    if(this->trading() == false)
        return false;

    if(0xFFFFFFFF - this->_you->money() < this->_money)
        return false;

    auto free_size = this->_you->items.free_size();
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        auto                item = this->_you->items[i];
        if(item == nullptr)
            continue;

        if(!(item->attr() & item::attrs::ITEM_ATTR_BUNDLE))
            continue;

        auto index = this->find(*item);
        if(index == 0xFF)
            continue;

        if(item->free_space() < this->_items[index]->count())
            return false;

        free_size++;
    }

    if(free_size < this->_items.size())
        return false;

    return true;
}

bool fb::game::trade::lock()
{
    try
    {
        if(this->trading() == false)
            throw std::runtime_error(message::trade::NOT_TRADING);

        this->_locked = true;
        if(this->_listener != nullptr)
        {
            this->_listener->on_trade_lock(this->_owner, true);
        }

        if(this->_you->trade._locked == false)  // 상대방이 이미 교환 확인을 누른 경우
        {
            this->_listener->on_trade_lock(*this->_you, false);
            return true;
        }
        else if(!this->flushable() || !this->_you->trade.flushable())   // 교환이 불가능한 경우
        {
            this->restore();
            this->_you->trade.restore();

            if(this->_listener != nullptr)
            {
                this->_listener->on_trade_failed(this->_owner);
                this->_listener->on_trade_failed(*this->_you);
            }
            this->end();
            return false;
        }
        else
        {
            this->flush();
            this->_you->trade.flush();

            if(this->_listener != nullptr)
            {
                this->_listener->on_trade_success(this->_owner);
                this->_listener->on_trade_success(*this->_you);
            }
            this->end();
            return true;
        }
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(this->_owner, e.what());

        return false;
    }
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
    catch(std::exception&)
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
    const auto              class_name = game::master::get().class2name(this->_owner.cls(), this->_owner.promotion());
    if(class_name == nullptr)
        return ostream;

    std::stringstream sstream;
    sstream << this->_owner.name() << '(' << class_name->c_str() << ')';

    ostream.write_u8(0x42)
        .write_u8(0x00)
        .write_u32(this->_owner.id())
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