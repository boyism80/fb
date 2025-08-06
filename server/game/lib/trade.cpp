#include <fb/game/character.h>
#include <fb/game/trade.h>

fb::game::trade::trade()
{ }

fb::game::trade::~trade()
{ }

void fb::game::trade::owner(std::shared_ptr<character> owner)
{
    this->_owner = owner->weak_from_this_as<character>();
}

std::shared_ptr<fb::game::character> fb::game::trade::owner() const
{
    return this->_owner.lock();
}

std::shared_ptr<fb::game::character> fb::game::trade::you() const
{
    return this->_you.lock();
}

bool fb::game::trade::begin(std::shared_ptr<fb::game::character> you)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    try
    {
        if (owner->id() == you->id())
        {
            // 자기 자신과 거래를 하려고 시도하는 경우
            return false;
        }

        if (owner->option(OPTION::TRADE) == false)
        {
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_REFUSED_BY_ME));
        }

        if (you->option(OPTION::TRADE) == false)
        {
            // 상대방이 교환 거부중
            std::stringstream sstream;
            sstream << you->name() << _TEXT(MESSAGE_TRADE_REFUSED_BY_PARTNER);
            throw std::runtime_error(sstream.str());
        }

        if (this->trading())
        {
            return false;
        }

        if (you->trade.trading())
        {
            // 상대방이 이미 교환중
            std::stringstream sstream;
            sstream << you->name() << _TEXT(MESSAGE_TRADE_PARTNER_ALREADY_TRADING);
            throw std::runtime_error(sstream.str());
        }

        if (owner->sight(*you) == false)
        {
            // 상대방이 시야에서 보이지 않음
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_PARTNER_INVISIBLE));
        }

        if (owner->distance_sqrt(*you) > 16)
        {
            // 상대방과의 거리가 너무 멈
            std::stringstream sstream;
            sstream << you->name() << _TEXT(MESSAGE_TRADE_PARTNER_TOO_FAR);

            throw std::runtime_error(sstream.str());
        }

        this->_you      = you->weak_from_this_as<character>();
        you->trade._you = owner->weak_from_this_as<character>();
        owner->listener.on_trade_begin(*owner, *you);
        you->listener.on_trade_begin(*you, *owner);

        return true;
    }
    catch (std::exception& e)
    {
        owner->message(e.what(), MESSAGE_TYPE::STATE);
    }
    return false;
}

void fb::game::trade::end()
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    auto you = this->_you.lock();
    if (you != nullptr)
    {
        auto& trade = you->trade;
        trade._you.reset();
        trade.end();
        this->_you.reset();
    }

    this->_locked = false;
    if (this->_money > 0)
    {
        owner->money_add(this->_money);
        this->_money = 0;
    }

    for (auto& [index, order] : this->_items)
    {
        auto item = owner->items[index];
        if (item == nullptr)
            continue;

        item->trade_count(0);
        owner->items.update(index);
    }
    this->_items.clear();
}

bool fb::game::trade::trading() const
{
    auto you = this->_you.lock();
    return you != nullptr;
}

bool fb::game::trade::up_item(uint8_t index)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    auto you = this->_you.lock();
    if (you == nullptr)
        return false;

    try
    {
        auto item = owner->items[index];
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
            owner->listener.on_trade_bundle(*owner);
        }
        else
        {
            // 일반 아이템의 거래 시도
            item->trade_count(1);
            auto order = this->add(index);
            if (order == 0xFF)
                return false;

            owner->items.update(index);
            owner->listener.on_trade_item(*owner, *you, order, *this->item(index));
        }

        return true;
    }
    catch (std::exception& e)
    {
        owner->message(e.what(), MESSAGE_TYPE::POPUP);
    }
    return false;
}

bool fb::game::trade::up_money(uint32_t money)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    auto you = this->_you.lock();
    if (you == nullptr)
        return false;

    try
    {
        this->_money = std::min<uint32_t>(owner->money(), money);
        owner->update(STATE_LEVEL::EXP_MONEY);
        owner->listener.on_trade_money(*owner, *you, this->_money);

        return true;
    }
    catch (std::exception& e)
    {
        owner->message(e.what(), MESSAGE_TYPE::POPUP);
    }

    return false;
}

uint32_t fb::game::trade::money() const
{
    return this->_money;
}

bool fb::game::trade::count(uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    auto you = this->_you.lock();
    if (you == nullptr)
        return false;

    try
    {
        if (this->trading() == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_TRADING));

        if (this->_selected == 0xFF)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_SELECTED));

        auto  item  = owner->items[this->_selected];
        auto& model = item->based<fb::model::item>();
        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE));

        auto remained = item->count() - item->trade_count();
        if (remained < count)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_INVALID_COUNT));

        item->trade_count(item->trade_count() + count);
        owner->items.update(this->_selected);

        auto order = this->add(this->_selected);
        owner->listener.on_trade_item(*owner, *you, order, *this->item(this->_selected));

        this->_selected = 0xFF;
        return true;
    }
    catch (std::exception& e)
    {
        owner->message(e.what(), MESSAGE_TYPE::POPUP);
    }
    return false;
}

bool fb::game::trade::cancel()
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    auto you = this->_you.lock();
    if (you == nullptr)
        return false;

    try
    {
        if (this->trading() == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_TRADING));

        this->restore();
        you->trade.restore();

        owner->listener.on_trade_cancel(*owner, *you);

        this->end();
        return true;
    }
    catch (std::exception& e)
    {
        this->end();
        owner->message(e.what(), MESSAGE_TYPE::POPUP);
        return false;
    }
}

uint8_t fb::game::trade::add(uint8_t index)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0xFF;

    if (this->_items.contains(index))
        return this->_items.at(index);

    auto order = this->_items.size();
    this->_items.insert({index, order});
    return order;
}

void fb::game::trade::restore()
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    for (auto& [index, order] : this->_items)
    {
        auto item = owner->items[index];
        if (item == nullptr)
            continue;

        item->trade_count(0);
        owner->items.update(index);
    }
    this->_items.clear();

    this->_money = 0;
    owner->update(STATE_LEVEL::EXP_MONEY);
}

std::shared_ptr<fb::game::item> fb::game::trade::find(const fb::model::item& item) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    for (auto& [index, order] : this->_items)
    {
        auto x = owner->items[index];
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
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        throw std::runtime_error("owner is nullptr");

    if (0xFFFFFFFF - trade.money() < owner->money())
        throw std::runtime_error(_TEXT(MESSAGE_MONEY_FULL));

    auto buffer = std::unordered_map<uint32_t, uint16_t>{};
    for (const auto& item : trade.items())
    {
        auto& model       = item->based<fb::model::item>();
        buffer[model.id] += item->trade_count();
    }

    if (!owner->items.is_rewardable(buffer))
        throw std::runtime_error(_TEXT(MESSAGE_ITEM_FULL));
}

void fb::game::trade::exchange(trade& trade1, trade& trade2)
{
    static auto push_buffer = [](trade& trade, std::vector<std::shared_ptr<fb::game::item>>& buffer) -> uint32_t {
        auto owner = trade._owner.lock();
        if (owner == nullptr)
            return 0;

        for (auto& [index, order] : trade._items)
        {
            auto item = owner->items[index];
            if (item == nullptr)
                continue;

            auto trade_count = item->trade_count();
            item->trade_count(0);

            auto split = item->split(trade_count);
            if (split == item)
                owner->items.remove(item, item->count(), ITEM_DELETE_TYPE::NONE, true);

            buffer.push_back(split);
        }

        auto money   = trade._money;
        trade._money = 0;
        owner->money_reduce(money);
        return money;
    };

    trade1.assert_exchange(trade2);
    trade2.assert_exchange(trade1);

    auto buffer1 = std::vector<std::shared_ptr<fb::game::item>>();
    auto money1  = push_buffer(trade1, buffer1);

    auto buffer2 = std::vector<std::shared_ptr<fb::game::item>>();
    auto money2  = push_buffer(trade2, buffer2);

    auto owner1 = trade1._owner.lock();
    if (owner1 == nullptr)
        return;

    auto owner2 = trade2._owner.lock();
    if (owner2 == nullptr)
        return;

    for (auto& item : buffer2)
    {
        owner1->items.add(item);
    }
    owner1->money_add(money2);

    for (auto& item : buffer1)
    {
        owner2->items.add(item);
    }
    owner2->money_add(money1);
}

bool fb::game::trade::lock()
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    auto you = this->_you.lock();
    if (you == nullptr)
        return false;

    try
    {
        if (this->trading() == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_TRADING));

        this->_locked = true;
        if (you->trade._locked == false) // 상대가 아직 OK 안누름
        {
            owner->listener.on_trade_lock(*owner, *you);

            return true;
        }
        else
        {
            this->exchange(*this, you->trade);

            // Call listener for packet response
            owner->listener.on_trade_success(*owner, *you);

            // Update state after successful trade
            owner->update(STATE_LEVEL::EXP_MONEY);
            you->update(STATE_LEVEL::EXP_MONEY);

            this->end();
            return true;
        }
    }
    catch (std::exception& e)
    {
        owner->listener.on_trade_failed(*owner, *you);

        this->end();
        return false;
    }
}

const std::vector<std::shared_ptr<fb::game::item>> fb::game::trade::items() const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return std::vector<std::shared_ptr<fb::game::item>>();

    auto result = std::vector<std::shared_ptr<fb::game::item>>();
    for (auto& [index, order] : this->_items)
    {
        auto item = owner->items[index];
        if (item == nullptr)
            continue;

        result.push_back(item);
    }
    return result;
}

const std::shared_ptr<fb::game::item> fb::game::trade::item(uint8_t index) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    if (this->_items.contains(index) == false)
        return nullptr;

    return owner->items[index];
}