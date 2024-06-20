#include <fb/game/session.h>
#include <fb/game/trade.h>
#include <fb/game/old_model.h>

fb::game::trade::trade(session& owner) : 
    _owner(owner)
{ }

fb::game::trade::~trade()
{ }

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
    auto listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        if(this->_owner.id() == you.id())
        {
            // 자기 자신과 거래를 하려고 시도하는 경우
            return false;
        }

        if(this->_owner.option(OPTION::TRADE) == false)
        {
            throw std::runtime_error(message::trade::REFUSED_BY_ME);
        }

        if(you.option(OPTION::TRADE) == false)
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
        if(listener != nullptr)
        {
            listener->on_trade_begin(this->_owner, you);
            listener->on_trade_begin(you, this->_owner);
        }

        return true;
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(this->_owner, e.what(), MESSAGE_TYPE::STATE);
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

bool fb::game::trade::up(fb::game::item& item)
{
    auto listener = this->_owner.get_listener<fb::game::session>();
    auto model = item.based<fb::game::item>();

    try
    {
        if(this->trading() == false)
            throw std::runtime_error(message::trade::NOT_TRADING);

        if(model->trade == false)
            throw std::runtime_error(message::trade::NOT_ALLOWED_TO_TRADE);

        if(enum_in(item.attr(), ITEM_ATTRIBUTE::BUNDLE) && item.count() > 1)
        {
            // 묶음 단위의 아이템 형식 거래 시도
            this->_selected = &item;
            if(listener != nullptr)
                listener->on_trade_bundle(this->_owner);
        }
        else
        {
            // 일반 아이템의 거래 시도
            auto splitted = this->_owner.items.remove(item);
            auto index = this->add(*splitted);
            if(index == 0xFF)
                return false;

            if(listener != nullptr)
                listener->on_trade_item(this->_owner, this->_owner, index);
            if(listener != nullptr)
                listener->on_trade_item(*this->_you, this->_owner, index);
        }

        return true;
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(this->_owner, e.what(), MESSAGE_TYPE::POPUP);
        return false;
    }
}

bool fb::game::trade::up(uint8_t money)
{
    auto listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        // 입력한 금전 양을 계속해서 빼면 안된다.
        // 100전 입력한 경우 -1, -10, -100 이렇게 까여버림
        auto                total = this->_owner.money() + this->_money;
        if(money > total)
            money = total;

        this->_owner.money(total - money);
        this->_money = money;

        if(listener != nullptr)
        {
            listener->on_trade_money(this->_owner, this->_owner);
            listener->on_trade_money(this->_owner, this->_owner);
        }
        
        return true;
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(this->_owner, e.what(), MESSAGE_TYPE::POPUP);

        return false;
    }
}

uint32_t fb::game::trade::money() const
{
    return this->_money;
}

bool fb::game::trade::count(uint16_t count)
{
    auto listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        if(this->trading() == false)
            throw std::runtime_error(message::trade::NOT_TRADING);

        if(this->_selected == nullptr)
            throw std::runtime_error(message::trade::NOT_SELECTED);

        auto model = _selected->based<fb::game::item>();
        if(model->trade == false)
            throw std::runtime_error(message::trade::NOT_ALLOWED_TO_TRADE);

        if(this->_selected->count() < count)
            throw std::runtime_error(message::trade::INVALID_COUNT);

        auto splitted = this->_owner.items.remove(*this->_selected, count);
        auto index = this->add(*splitted);

        if(listener != nullptr)
        {
            listener->on_trade_item(this->_owner, this->_owner, index);
            listener->on_trade_item(*this->_you, this->_owner, index);
        }

        this->_selected = nullptr;
        return true;
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(this->_owner, e.what(), MESSAGE_TYPE::POPUP);
        return false;
    }
}

bool fb::game::trade::cancel()
{
    auto listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        if(this->trading() == false)
            throw std::runtime_error(message::trade::NOT_TRADING);

        this->restore();
        this->_you->trade.restore();

        if(listener != nullptr)
        {
            listener->on_trade_cancel(this->_owner, this->_owner);
            listener->on_trade_cancel(*this->_you, this->_owner);
        }

        this->end();
        return true;
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(this->_owner, e.what(), MESSAGE_TYPE::POPUP);
        return false;
    }
}

uint8_t fb::game::trade::add(fb::game::item& item)
{
    if(enum_in(item.attr(), ITEM_ATTRIBUTE::BUNDLE))
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
    this->_owner.items.add(this->_items);
    this->_items.clear();

    this->_owner.money_add(this->_money);
    this->_money = 0;
}

void fb::game::trade::flush()
{
    this->_you->items.add(this->_items);
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
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto                item = this->_you->items[i];
        if(item == nullptr)
            continue;

        if(enum_in(item->attr(), ITEM_ATTRIBUTE::BUNDLE) == false)
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
    auto listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        if(this->trading() == false)
            throw std::runtime_error(message::trade::NOT_TRADING);

        this->_locked = true;
        if(listener != nullptr)
            listener->on_trade_lock(this->_owner, true);

        if(this->_you->trade._locked == false)  // 상대방이 이미 교환 확인을 누른 경우
        {
            if(listener != nullptr)
                listener->on_trade_lock(*this->_you, false);
            return true;
        }
        else if(!this->flushable() || !this->_you->trade.flushable())   // 교환이 불가능한 경우
        {
            this->restore();
            this->_you->trade.restore();

            if(listener != nullptr)
            {
                listener->on_trade_failed(this->_owner);
                listener->on_trade_failed(*this->_you);
            }
            this->end();
            return false;
        }
        else
        {
            this->flush();
            this->_you->trade.flush();

            if(listener != nullptr)
            {
                listener->on_trade_success(this->_owner);
                listener->on_trade_success(*this->_you);
            }
            this->end();
            return true;
        }
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(this->_owner, e.what(), MESSAGE_TYPE::STATE);
        return false;
    }
}

const std::vector<fb::game::item*>& fb::game::trade::items() const
{
    return this->_items;
}

const fb::game::item* fb::game::trade::item(uint8_t index) const
{
    return this->_items.at(index);
}