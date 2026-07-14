#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/trade.h>
#include <fb/model/model.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <tuple>

using namespace fb::game;

trade::trade()
{ }

trade::~trade()
{ }

void trade::owner(std::shared_ptr<character> owner)
{
    this->_owner = owner->weak_from_this_as<character>();
}

std::shared_ptr<fb::game::character> trade::owner() const
{
    return this->_owner.lock();
}

std::shared_ptr<fb::game::character> trade::you() const
{
    return this->_you.lock();
}

async::task<bool> trade::begin(std::shared_ptr<fb::game::character> you)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return false;

    auto error = std::optional<std::string>{};
    try
    {
        // Try to trade with yourself
        if (owner->id == you->id)
            co_return false;

        // The owner has refused the trade
        if (owner->option(OPTION::TRADE) == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_REFUSED_BY_ME));

        // The partner has refused the trade
        if (you->option(OPTION::TRADE) == false)
            throw std::runtime_error(std::format(_TEXT(MESSAGE_TRADE_REFUSED_BY_PARTNER), you->name()));

        // The trade is already in progress
        if (this->trading())
            co_return false;

        // The partner is already trading
        if (you->trade.trading())
            throw std::runtime_error(std::format(_TEXT(MESSAGE_TRADE_PARTNER_ALREADY_TRADING), you->name()));

        // The partner is not in sight
        if (owner->sight(*you) == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_PARTNER_INVISIBLE));

        // The distance to the partner is too far
        if (owner->distance_sqrt(*you) > 16)
            throw std::runtime_error(std::format(_TEXT(MESSAGE_TRADE_PARTNER_TOO_FAR), you->name()));

        this->_you      = you->weak_from_this_as<character>();
        you->trade._you = owner->weak_from_this_as<character>();
        co_await owner->listener.on_trade_begin(*owner, *you);
        co_await you->listener.on_trade_begin(*you, *owner);

        co_return true;
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
        co_await owner->message(error.value(), MESSAGE_TYPE::STATE);
    co_return false;
}

async::task<void> trade::end()
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return;

    auto you = this->_you.lock();
    if (you != nullptr)
    {
        auto& trade = you->trade;
        trade._you.reset();
        co_await trade.end();
        this->_you.reset();
    }

    this->_locked = false;
    if (this->_money > 0)
    {
        std::ignore  = co_await owner->money_add(this->_money);
        this->_money = 0;
    }

    for (auto& [index, order] : this->_items)
    {
        auto item = owner->items[index];
        if (item == nullptr)
            continue;

        item->trade_count(0);
        std::ignore = co_await owner->items.update(index);
    }
    this->_items.clear();
}

bool trade::trading() const
{
    auto you = this->_you.lock();
    return you != nullptr;
}

async::task<bool> trade::up_item(uint8_t index)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return false;

    auto you = this->_you.lock();
    if (you == nullptr)
        co_return false;

    auto error = std::optional<std::string>{};
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
            // Attempt to trade bundle-type item
            this->_selected = index;
            co_await owner->listener.on_trade_bundle(*owner);
            co_return true;
        }

        // Attempt to trade single item
        item->trade_count(1);
        auto order = this->add(index);
        if (order == 0xFF)
            co_return false;

        std::ignore = co_await owner->items.update(index);
        co_await owner->listener.on_trade_item(*owner, *you, order, *this->item(index));
        co_return true;
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
        co_await owner->message(error.value(), MESSAGE_TYPE::POPUP);
    co_return false;
}

async::task<bool> trade::up_money(uint32_t money)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return false;

    auto you = this->_you.lock();
    if (you == nullptr)
        co_return false;

    auto error = std::optional<std::string>{};
    try
    {
        this->_money = std::min<uint32_t>(owner->money(), money);
        co_await owner->update(UPDATE_STATE_LEVEL::EXP_MONEY);
        co_await owner->listener.on_trade_money(*owner, *you, this->_money);

        co_return true;
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
        co_await owner->message(error.value(), MESSAGE_TYPE::POPUP);
    co_return false;
}

uint32_t trade::money() const
{
    return this->_money;
}

async::task<bool> trade::count(uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return false;

    auto you = this->_you.lock();
    if (you == nullptr)
        co_return false;

    auto error = std::optional<std::string>{};
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
        std::ignore = co_await owner->items.update(this->_selected);

        auto order = this->add(this->_selected);
        co_await owner->listener.on_trade_item(*owner, *you, order, *this->item(this->_selected));

        this->_selected = 0xFF;
        co_return true;
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
        co_await owner->message(error.value(), MESSAGE_TYPE::POPUP);
    co_return false;
}

async::task<bool> trade::cancel()
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return false;

    auto you = this->_you.lock();
    if (you == nullptr)
        co_return false;

    auto error = std::optional<std::string>{};
    try
    {
        if (this->trading() == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_TRADING));

        co_await this->restore();
        co_await you->trade.restore();

        co_await owner->listener.on_trade_cancel(*owner, *you);

        co_await this->end();
        co_return true;
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    co_await this->end();
    if (error.has_value())
        co_await owner->message(error.value(), MESSAGE_TYPE::POPUP);
    co_return false;
}

uint8_t trade::add(uint8_t index)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0xFF;

    if (this->_items.contains(index))
        return this->_items.at(index);

    auto order = static_cast<uint8_t>(this->_items.size());
    this->_items.insert({index, order});
    return order;
}

async::task<void> trade::restore()
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return;

    for (auto& [index, order] : this->_items)
    {
        auto item = owner->items[index];
        if (item == nullptr)
            continue;

        item->trade_count(0);
        std::ignore = co_await owner->items.update(index);
    }
    this->_items.clear();

    this->_money = 0;
    co_await owner->update(UPDATE_STATE_LEVEL::EXP_MONEY);
}

std::shared_ptr<fb::game::item> trade::find(const fb::model::item& item) const
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

void trade::assert_exchange(const trade& trade) const
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

    for (auto& item : owner->items)
    {
        if (item == nullptr)
            continue;

        if (item->trade_count() == 0)
            continue;

        auto& model = item->based<fb::model::item>();
        if (buffer.contains(model.id) == false)
            continue;

        if (buffer[model.id] > item->count())
            buffer[model.id] -= item->count();
        else
            buffer.erase(model.id);
    }

    if (!owner->items.is_rewardable(buffer))
        throw std::runtime_error(_TEXT(MESSAGE_ITEM_FULL));
}

async::task<void> trade::exchange(trade& trade1, trade& trade2)
{
    static auto push_buffer = [](trade&                                        trade,
                                 std::vector<std::shared_ptr<fb::game::item>>& buffer) -> async::task<uint32_t> {
        auto owner = trade._owner.lock();
        if (owner == nullptr)
            co_return 0;

        for (auto& [index, order] : trade._items)
        {
            auto item = owner->items[index];
            if (item == nullptr)
                continue;

            auto trade_count = item->trade_count();
            item->trade_count(0);

            auto split = item->split(trade_count);
            if (split == item)
                std::ignore = co_await owner->items.remove(item, item->count(), ITEM_DELETE_TYPE::NONE, true);

            buffer.push_back(split);
        }

        auto money   = trade._money;
        trade._money = 0;
        owner->money_reduce(money);
        co_return money;
    };

    trade1.assert_exchange(trade2);
    trade2.assert_exchange(trade1);

    auto buffer1 = std::vector<std::shared_ptr<fb::game::item>>();
    auto money1  = co_await push_buffer(trade1, buffer1);

    auto buffer2 = std::vector<std::shared_ptr<fb::game::item>>();
    auto money2  = co_await push_buffer(trade2, buffer2);

    auto owner1 = trade1._owner.lock();
    if (owner1 == nullptr)
        co_return;

    auto owner2 = trade2._owner.lock();
    if (owner2 == nullptr)
        co_return;

    for (auto& item : buffer2)
    {
        std::ignore = co_await owner1->items.add(item);
    }
    std::ignore = co_await owner1->money_add(money2);

    for (auto& item : buffer1)
    {
        std::ignore = co_await owner2->items.add(item);
    }
    std::ignore = co_await owner2->money_add(money1);
}

async::task<bool> trade::lock()
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return false;

    auto you = this->_you.lock();
    if (you == nullptr)
        co_return false;

    auto failed = false;
    try
    {
        if (this->trading() == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_TRADING));

        this->_locked = true;
        if (you->trade._locked == false) // Peer has not confirmed yet
        {
            co_await owner->listener.on_trade_lock(*owner, *you);
            co_return true;
        }

        co_await this->exchange(*this, you->trade);

        // Call listener for packet response
        co_await owner->listener.on_trade_success(*owner, *you);

        // Update state after successful trade
        co_await owner->update(UPDATE_STATE_LEVEL::EXP_MONEY);
        co_await you->update(UPDATE_STATE_LEVEL::EXP_MONEY);

        // Log trade completion
        auto log_data               = Json::Value();
        log_data["character1_id"]   = static_cast<Json::Int64>(owner->id);
        log_data["character1_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
        log_data["character2_id"]   = static_cast<Json::Int64>(you->id);
        log_data["character2_name"] = UTF8(you->name(), PLATFORM::WINDOWS);
        log_data["money1"]          = static_cast<Json::Int64>(this->_money);
        log_data["money2"]          = static_cast<Json::Int64>(you->trade._money);
        auto items1                 = std::vector<Json::Value>();
        for (auto& [index, order] : this->_items)
        {
            auto item = owner->items[index];
            if (item != nullptr)
            {
                auto item_data         = Json::Value{};
                item_data["item_id"]   = static_cast<Json::Int64>(item->based<fb::model::item>().id);
                item_data["item_name"] = UTF8(item->name(), PLATFORM::WINDOWS);
                item_data["count"]     = static_cast<Json::Int64>(item->trade_count());
                items1.push_back(item_data);
            }
        }
        log_data["items1"] = Json::Value(Json::arrayValue);
        for (auto& item : items1)
        {
            log_data["items1"].append(item);
        }
        auto items2 = std::vector<Json::Value>();
        for (auto& [index, order] : you->trade._items)
        {
            auto item = you->items[index];
            if (item != nullptr)
            {
                auto item_data         = Json::Value{};
                item_data["item_id"]   = static_cast<Json::Int64>(item->based<fb::model::item>().id);
                item_data["item_name"] = UTF8(item->name(), PLATFORM::WINDOWS);
                item_data["count"]     = static_cast<Json::Int64>(item->trade_count());
                items2.push_back(item_data);
            }
        }
        log_data["items2"] = Json::Value(Json::arrayValue);
        for (auto& item : items2)
        {
            log_data["items2"].append(item);
        }
        owner->server.log.write("trade_complete", log_data);

        co_await this->end();
        co_return true;
    }
    catch (std::exception& e)
    {
        std::ignore = e;
        failed      = true;
    }

    if (failed)
    {
        co_await owner->listener.on_trade_failed(*owner, *you);
        co_await this->end();
    }
    co_return false;
}

const std::vector<std::shared_ptr<fb::game::item>> trade::items() const
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

const std::shared_ptr<fb::game::item> trade::item(uint8_t index) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    if (this->_items.contains(index) == false)
        return nullptr;

    return owner->items[index];
}
