#include <fb/game/character.h>
#include <fb/game/trade.h>

fb::game::trade::trade(character& owner) :
    _owner(owner)
{ }

fb::game::trade::~trade()
{ }

fb::game::character* fb::game::trade::you() const
{
    return this->_you;
}

bool fb::game::trade::begin(fb::game::character& you)
{
    try
    {
        if (this->_owner.id() == you.id())
        {
            // 자기 자신과 거래를 하려고 시도하는 경우
            return false;
        }

        if (this->_owner.option(OPTION::TRADE) == false)
        {
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_REFUSED_BY_ME));
        }

        if (you.option(OPTION::TRADE) == false)
        {
            // 상대방이 교환 거부중
            std::stringstream sstream;
            sstream << you.name() << _TEXT(MESSAGE_TRADE_REFUSED_BY_PARTNER);
            throw std::runtime_error(sstream.str());
        }

        if (this->trading())
        {
            return false;
        }

        if (you.trade.trading())
        {
            // 상대방이 이미 교환중
            std::stringstream sstream;
            sstream << you.name() << _TEXT(MESSAGE_TRADE_PARTNER_ALREADY_TRADING);
            throw std::runtime_error(sstream.str());
        }

        if (this->_owner.sight(you) == false)
        {
            // 상대방이 시야에서 보이지 않음
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_PARTNER_INVISIBLE));
        }

        if (this->_owner.distance_sqrt(you) > 16)
        {
            // 상대방과의 거리가 너무 멈
            std::stringstream sstream;
            sstream << you.name() << _TEXT(MESSAGE_TRADE_PARTNER_TOO_FAR);

            throw std::runtime_error(sstream.str());
        }

        this->_you     = &you;
        you.trade._you = &this->_owner;
        this->_owner.listener.on_trade_begin(this->_owner, you);
        this->_owner.listener.on_trade_begin(you, this->_owner);

        return true;
    }
    catch (std::exception& e)
    {
        this->_owner.message(e.what(), MESSAGE_TYPE::STATE);
    }
    return false;
}

void fb::game::trade::end()
{
    if (this->_you != nullptr)
    {
        auto& trade = this->_you->trade;
        trade._you  = nullptr;
        trade.end();
        this->_you = nullptr;
    }

    this->_locked = false;
    if (this->_money > 0)
    {
        this->_owner.money_add(this->_money);
        this->_money = 0;
    }

    for (auto& [index, order] : this->_items)
    {
        auto item = this->_owner.items[index];
        if (item == nullptr)
            continue;

        item->trade_count(0);
        this->_owner.items.update(index);
    }
    this->_items.clear();
}

bool fb::game::trade::trading() const
{
    return this->_you != nullptr;
}

bool fb::game::trade::up_item(uint8_t index)
{
    try
    {
        auto item = this->_owner.items[index];
        if (item == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_FOUND_ITEM));

        auto& model = item->based<fb::model::item>();

        if (this->trading() == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_TRADING));

        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE));

        if (ENUM_IN(model.attr(), ITEM_ATTRIBUTE::BUNDLE) && item->count() > 1)
        {
            // 묶음 단위의 아이템 형식 거래 시도
            this->_selected = index;
            this->_owner.listener.on_trade_bundle(this->_owner);
        }
        else
        {
            // 일반 아이템의 거래 시도
            item->trade_count(1);
            auto order = this->add(index);
            if (order == 0xFF)
                return false;

            this->_owner.items.update(index);
            this->_owner.listener.on_trade_item(this->_owner, *this->_you, order, *this->item(index));
        }

        return true;
    }
    catch (std::exception& e)
    {
        this->_owner.message(e.what(), MESSAGE_TYPE::POPUP);
    }
    return false;
}

bool fb::game::trade::up_money(uint32_t money)
{

    try
    {
        this->_money = std::min<uint32_t>(this->_owner.money(), money);
        this->_owner.update(STATE_LEVEL::EXP_MONEY);
        this->_owner.listener.on_trade_money(this->_owner, *this->_you, this->_money);

        return true;
    }
    catch (std::exception& e)
    {
        this->_owner.message(e.what(), MESSAGE_TYPE::POPUP);
    }

    return false;
}

uint32_t fb::game::trade::money() const
{
    return this->_money;
}

bool fb::game::trade::count(uint16_t count)
{

    try
    {
        if (this->trading() == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_TRADING));

        if (this->_selected == 0xFF)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_SELECTED));

        auto  item  = this->_owner.items[this->_selected];
        auto& model = item->based<fb::model::item>();
        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE));

        auto remained = item->count() - item->trade_count();
        if (remained < count)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_INVALID_COUNT));

        item->trade_count(item->trade_count() + count);
        this->_owner.items.update(this->_selected);

        auto order = this->add(this->_selected);
        this->_owner.listener.on_trade_item(this->_owner, *this->_you, order, *this->item(this->_selected));

        this->_selected = 0xFF;
        return true;
    }
    catch (std::exception& e)
    {
        this->_owner.message(e.what(), MESSAGE_TYPE::POPUP);
    }
    return false;
}

bool fb::game::trade::cancel()
{
    try
    {
        if (this->trading() == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_TRADING));

        this->restore();
        this->_you->trade.restore();

        this->_owner.listener.on_trade_cancel(this->_owner, *this->_you);

        this->end();
        return true;
    }
    catch (std::exception& e)
    {
        this->end();
        this->_owner.message(e.what(), MESSAGE_TYPE::POPUP);
        return false;
    }
}

uint8_t fb::game::trade::add(uint8_t index)
{
    if (this->_items.contains(index))
        return this->_items.at(index);

    auto order = this->_items.size();
    this->_items.insert({index, order});
    return order;
}

void fb::game::trade::restore()
{
    for (auto& [index, order] : this->_items)
    {
        auto item = this->_owner.items[index];
        if (item == nullptr)
            continue;

        item->trade_count(0);
        this->_owner.items.update(index);
    }
    this->_items.clear();

    this->_money = 0;
    this->_owner.update(STATE_LEVEL::EXP_MONEY);
}

std::shared_ptr<fb::game::item> fb::game::trade::find(const fb::model::item& item) const
{
    for (auto& [index, order] : this->_items)
    {
        auto x = this->_owner.items[index];
        if (x == nullptr)
            continue;

        auto& model = x->based<fb::model::item>();
        if (&model == &item)
            return std::static_pointer_cast<fb::game::item>(x);
    }

    return nullptr;
}

void fb::game::trade::assert_exchange(const fb::game::trade& trade) const
{
    if (0xFFFFFFFF - trade.money() < this->_owner.money())
        throw std::runtime_error(_TEXT(MESSAGE_MONEY_FULL));

    auto free_size = this->_owner.items.free_size();
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->_owner.items[i];
        if (item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        if (ENUM_IN(model.attr(), ITEM_ATTRIBUTE::BUNDLE) == false)
            continue;

        auto found = trade.find(model);
        if (found == nullptr)
            continue;

        if (model.capacity < item->count() + found->trade_count() - item->trade_count())
            continue;

        free_size++;
    }

    if (free_size < trade.items().size())
        throw std::runtime_error(_TEXT(MESSAGE_ITEM_FULL));
}

void fb::game::trade::exchange(trade& trade1, trade& trade2)
{
    static auto push_buffer = [](trade& trade, std::vector<std::shared_ptr<fb::game::item>>& buffer) {
        for (auto& [index, order] : trade._items)
        {
            auto item = trade._owner.items[index];
            if (item == nullptr)
                continue;

            auto trade_count = item->trade_count();
            item->trade_count(0);

            auto split = item->split(trade_count);
            if (split == item)
                trade._owner.items.remove(item);

            buffer.push_back(split);
        }

        auto money   = trade._money;
        trade._money = 0;
        trade._owner.money_reduce(money);
        return money;
    };

    trade1.assert_exchange(trade2);
    trade2.assert_exchange(trade1);

    auto buffer1 = std::vector<std::shared_ptr<fb::game::item>>();
    auto money1  = push_buffer(trade1, buffer1);

    auto buffer2 = std::vector<std::shared_ptr<fb::game::item>>();
    auto money2  = push_buffer(trade2, buffer2);

    for (auto& item : buffer2)
    {
        trade1._owner.items.add(item);
    }
    trade1._owner.money_add(money2);

    for (auto& item : buffer1)
    {
        trade2._owner.items.add(item);
    }
    trade2._owner.money_add(money1);
}

bool fb::game::trade::lock()
{
    try
    {
        if (this->trading() == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_TRADING));

        this->_locked = true;
        if (this->_you->trade._locked == false) // 상대가 아직 OK 안누름
        {
            this->_owner.listener.on_trade_lock(this->_owner, *this->_you);

            return true;
        }
        else
        {
            this->exchange(*this, this->_you->trade);
            this->_owner.listener.on_trade_success(this->_owner, *this->_you);

            this->end();
            return true;
        }
    }
    catch (std::exception& e)
    {
        this->_owner.listener.on_trade_failed(this->_owner, *this->_you);

        this->end();
        return false;
    }
}

const std::vector<std::shared_ptr<fb::game::item>> fb::game::trade::items() const
{
    auto result = std::vector<std::shared_ptr<fb::game::item>>();
    for (auto& [index, order] : this->_items)
    {
        auto item = this->_owner.items[index];
        if (item == nullptr)
            continue;

        result.push_back(item);
    }
    return result;
}

const std::shared_ptr<fb::game::item> fb::game::trade::item(uint8_t index) const
{
    if (this->_items.contains(index) == false)
        return nullptr;

    return this->_owner.items[index];
}