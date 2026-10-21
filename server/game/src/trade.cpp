#include <fb/game/trade.h>

#include <fb/encoding.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/model/model.h>

#include <json/json.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <format>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace fb::game;

trade::trade(character& me, character& you) :
    _sides{side{.owner = me.weak_from_this_as<character>()}, side{.owner = you.weak_from_this_as<character>()}}
{ }

trade::~trade()
{ }

size_t trade::index(const character& ch) const
{
    return this->_sides[0].owner.lock().get() == &ch ? 0 : 1;
}

bool trade::locked() const
{
    return this->_sides[0].locked || this->_sides[1].locked;
}

bool trade::begin(character& me, character& you)
{
    try
    {
        // Try to trade with yourself
        if (me.id == you.id)
            return false;

        if (you.hidden(me))
            return false;

        // The owner has refused the trade
        if (me.option(OPTION::TRADE) == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_REFUSED_BY_ME));

        // The partner has refused the trade
        if (you.option(OPTION::TRADE) == false)
            throw std::runtime_error(std::format(_TEXT(MESSAGE_TRADE_REFUSED_BY_PARTNER), you.name()));

        // The trade is already in progress
        if (me.trade != nullptr)
            return false;

        // The partner is already trading
        if (you.trade != nullptr)
            throw std::runtime_error(std::format(_TEXT(MESSAGE_TRADE_PARTNER_ALREADY_TRADING), you.name()));

        // The partner is not in sight
        if (me.sight(you) == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_PARTNER_INVISIBLE));

        // The distance to the partner is too far
        if (me.distance_sqrt(you) > 16)
            throw std::runtime_error(std::format(_TEXT(MESSAGE_TRADE_PARTNER_TOO_FAR), you.name()));

        auto session = std::make_shared<trade>(me, you);
        me.trade     = session;
        you.trade    = session;
        me.listener.on_trade_begin(me, you);
        you.listener.on_trade_begin(you, me);

        return true;
    }
    catch (std::exception& e)
    {
        me.message(e.what(), MESSAGE_TYPE::STATE);
    }
    return false;
}

void trade::restore()
{
    for (auto& side : this->_sides)
    {
        auto owner = side.owner.lock();
        if (owner == nullptr)
            continue;

        for (auto& [index, order] : side.items)
        {
            auto item = owner->items[index];
            if (item == nullptr)
                continue;

            item->trade_count(0);
            owner->items.update(index);
        }
        side.items.clear();

        // The offered money never left the owner's balance, so there is nothing to refund.
        side.money    = 0;
        side.selected = 0xFF;
        side.locked   = false;
        owner->update(UPDATE_STATE_LEVEL::EXP_MONEY);
    }
}

void trade::end()
{
    // Releasing the last character reference destroys this session.
    auto self = this->shared_from_this();
    for (auto& side : this->_sides)
    {
        auto owner = side.owner.lock();
        if (owner != nullptr && owner->trade == self)
            owner->trade.reset();
    }
}

bool trade::up_item(character& ch, uint8_t index)
{
    auto  i   = this->index(ch);
    auto& me  = this->_sides[i];
    auto  you = this->_sides[1 - i].owner.lock();
    if (you == nullptr)
        return false;

    try
    {
        auto item = ch.items[index];
        if (item == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_FOUND_ITEM));

        auto& model = item->model();

        if (this->locked())
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_ALREADY_LOCKED));

        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE));

        if (ENUM_IN(model.attr(), ITEM_ATTRIBUTE::BUNDLE) && item->count() > 1)
        {
            // Attempt to trade bundle-type item
            me.selected = index;
            ch.listener.on_trade_bundle(ch);
            return true;
        }

        // Attempt to trade single item
        item->trade_count(1);
        auto order = this->add(me, index);
        ch.items.update(index);
        ch.listener.on_trade_item(ch, *you, order, *item);
        return true;
    }
    catch (std::exception& e)
    {
        ch.message(e.what(), MESSAGE_TYPE::POPUP);
    }
    return false;
}

bool trade::up_money(character& ch, uint64_t money)
{
    auto  i   = this->index(ch);
    auto& me  = this->_sides[i];
    auto  you = this->_sides[1 - i].owner.lock();
    if (you == nullptr)
        return false;

    try
    {
        if (this->locked())
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_ALREADY_LOCKED));

        me.money = std::min<uint64_t>(ch.money(), money);
        ch.update(UPDATE_STATE_LEVEL::EXP_MONEY);
        ch.listener.on_trade_money(ch, *you, me.money);
        return true;
    }
    catch (std::exception& e)
    {
        ch.message(e.what(), MESSAGE_TYPE::POPUP);
    }
    return false;
}

uint64_t trade::money(const character& ch) const
{
    return this->_sides[this->index(ch)].money;
}

bool trade::count(character& ch, uint16_t count)
{
    auto  i   = this->index(ch);
    auto& me  = this->_sides[i];
    auto  you = this->_sides[1 - i].owner.lock();
    if (you == nullptr)
        return false;

    try
    {
        if (this->locked())
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_ALREADY_LOCKED));

        if (me.selected == 0xFF)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_SELECTED));

        auto item = ch.items[me.selected];
        if (item == nullptr)
        {
            me.selected = 0xFF;
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_SELECTED));
        }

        auto& model = item->model();
        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE));

        auto remained = item->count() - item->trade_count();
        if (count == 0 || remained < count)
            throw std::runtime_error(_TEXT(MESSAGE_TRADE_INVALID_COUNT));

        item->trade_count(item->trade_count() + count);
        ch.items.update(me.selected);

        auto order = this->add(me, me.selected);
        ch.listener.on_trade_item(ch, *you, order, *item);

        me.selected = 0xFF;
        return true;
    }
    catch (std::exception& e)
    {
        ch.message(e.what(), MESSAGE_TYPE::POPUP);
    }
    return false;
}

bool trade::cancel(character& ch)
{
    auto you = this->_sides[1 - this->index(ch)].owner.lock();

    this->restore();
    if (you != nullptr)
        ch.listener.on_trade_cancel(ch, *you);
    this->end();
    return true;
}

uint8_t trade::add(side& side, uint8_t index)
{
    if (side.items.contains(index))
        return side.items.at(index);

    auto order = static_cast<uint8_t>(side.items.size());
    side.items.insert({index, order});
    return order;
}

void trade::assert_exchange(size_t i) const
{
    auto& me  = this->_sides[i];
    auto& you = this->_sides[1 - i];

    auto owner = me.owner.lock();
    if (owner == nullptr)
        throw std::runtime_error("owner is nullptr");

    auto partner = you.owner.lock();
    if (partner == nullptr)
        throw std::runtime_error("partner is nullptr");

    // The offer is not escrowed; it must still be fully covered at exchange time.
    if (owner->money() < me.money)
        throw exchange_error(i, _TEXT(MESSAGE_NOT_ENOUGH_MONEY));

    for (auto& [index, order] : me.items)
    {
        auto item = owner->items[index];
        if (item == nullptr || item->trade_count() == 0 || item->count() < item->trade_count())
            throw exchange_error(i, _TEXT(MESSAGE_TRADE_FAILED));
    }

    if (std::numeric_limits<uint64_t>::max() - you.money < owner->money())
        throw exchange_error(i, _TEXT(MESSAGE_MONEY_FULL));

    auto incoming = std::unordered_map<uint32_t, uint32_t>{};
    for (auto& [index, order] : you.items)
    {
        auto item = partner->items[index];
        if (item == nullptr)
            continue;

        incoming[item->model().id] += item->trade_count();
    }

    // Capacity is checked against the inventory as it will be after this side's offer leaves.
    auto free_slots     = static_cast<int>(owner->items.free_size());
    auto bundle_remains = std::unordered_map<uint32_t, uint32_t>{};
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item  = owner->items.at(static_cast<uint8_t>(i));
        auto held  = owner->items.escrow().at(static_cast<uint8_t>(i));
        auto shown = item != nullptr ? item : held;
        if (shown == nullptr)
            continue;

        auto remains = static_cast<uint32_t>(item != nullptr ? item->count() - item->trade_count() : 0) +
                       static_cast<uint32_t>(held != nullptr ? held->count() : 0);
        if (remains == 0)
        {
            free_slots++;
            continue;
        }

        auto& model = shown->model();
        if (model.attr(ITEM_ATTRIBUTE::BUNDLE))
            bundle_remains.try_emplace(model.id, remains);
    }

    auto required_slots = 0;
    for (auto& [id, count] : incoming)
    {
        auto& model = fb::model::table::item[id];
        if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
        {
            required_slots += count;
        }
        else if (bundle_remains.contains(id))
        {
            if (model.capacity < bundle_remains[id] + count)
                throw exchange_error(i, _TEXT(MESSAGE_ITEM_FULL));
        }
        else
        {
            required_slots++;
        }
    }

    if (free_slots < required_slots)
        throw exchange_error(i, _TEXT(MESSAGE_ITEM_FULL));
}

async::task<void> trade::exchange()
{
    static auto push_buffer = [](side& side, std::vector<std::shared_ptr<fb::game::item>>& buffer) -> uint64_t {
        auto owner = side.owner.lock();
        if (owner == nullptr)
            return 0;

        for (auto& [index, order] : side.items)
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

        auto money = side.money;
        side.money = 0;
        owner->money_reduce(money);
        return money;
    };

    this->assert_exchange(0);
    this->assert_exchange(1);

    auto buffer1 = std::vector<std::shared_ptr<fb::game::item>>();
    auto money1  = push_buffer(this->_sides[0], buffer1);

    auto buffer2 = std::vector<std::shared_ptr<fb::game::item>>();
    auto money2  = push_buffer(this->_sides[1], buffer2);

    auto owner1 = this->_sides[0].owner.lock();
    if (owner1 == nullptr)
        co_return;

    auto owner2 = this->_sides[1].owner.lock();
    if (owner2 == nullptr)
        co_return;

    // Merging into an existing stack empties the moved item, so the log is built before the items are added.
    auto log_data               = Json::Value();
    log_data["character1_id"]   = static_cast<Json::Int64>(owner1->id);
    log_data["character1_name"] = UTF8(owner1->name(), PLATFORM::WINDOWS);
    log_data["character2_id"]   = static_cast<Json::Int64>(owner2->id);
    log_data["character2_name"] = UTF8(owner2->name(), PLATFORM::WINDOWS);
    log_data["money1"]          = static_cast<Json::Int64>(money1);
    log_data["money2"]          = static_cast<Json::Int64>(money2);
    log_data["items1"]          = Json::Value(Json::arrayValue);
    for (auto& item : buffer1)
    {
        auto item_data         = Json::Value{};
        item_data["item_id"]   = static_cast<Json::Int64>(item->model().id);
        item_data["item_name"] = UTF8(item->name(), PLATFORM::WINDOWS);
        item_data["count"]     = static_cast<Json::Int64>(item->count());
        if (item->uid() != 0)
            item_data["item_uid"] = static_cast<Json::UInt64>(item->uid());
        log_data["items1"].append(item_data);
    }
    log_data["items2"] = Json::Value(Json::arrayValue);
    for (auto& item : buffer2)
    {
        auto item_data         = Json::Value{};
        item_data["item_id"]   = static_cast<Json::Int64>(item->model().id);
        item_data["item_name"] = UTF8(item->name(), PLATFORM::WINDOWS);
        item_data["count"]     = static_cast<Json::Int64>(item->count());
        if (item->uid() != 0)
            item_data["item_uid"] = static_cast<Json::UInt64>(item->uid());
        log_data["items2"].append(item_data);
    }

    for (auto& item : buffer2)
    {
        std::ignore = co_await owner1->items.add(std::vector<std::shared_ptr<fb::game::item>>{item}, true);
        if (item->empty() == false && owner1->items.index(item) == 0xFF)
            std::ignore = co_await owner2->items.add(item);
    }
    owner1->money_add(money2);

    for (auto& item : buffer1)
    {
        std::ignore = co_await owner2->items.add(std::vector<std::shared_ptr<fb::game::item>>{item}, true);
        if (item->empty() == false && owner2->items.index(item) == 0xFF)
            std::ignore = co_await owner1->items.add(item);
    }
    owner2->money_add(money1);

    owner1->server.log.write("trade_complete", log_data);
}

async::task<bool> trade::lock(character& ch)
{
    // end() releases both character references while this coroutine still runs.
    auto self = this->shared_from_this();
    auto i    = this->index(ch);
    auto you  = this->_sides[1 - i].owner.lock();
    if (you == nullptr)
        co_return false;

    this->_sides[i].locked = true;
    if (this->_sides[1 - i].locked == false) // Peer has not confirmed yet
    {
        ch.listener.on_trade_lock(ch, *you);
        co_return true;
    }

    try
    {
        co_await this->exchange();

        ch.listener.on_trade_success(ch, *you);
        this->restore();
        this->end();
        co_return true;
    }
    catch (std::exception& e)
    {
        auto me_message  = std::string(_TEXT(MESSAGE_TRADE_FAILED));
        auto you_message = me_message;
        auto failure     = dynamic_cast<const exchange_error*>(&e);
        if (failure != nullptr)
        {
            if (failure->side == i)
                me_message = failure->what();
            else
                you_message = failure->what();
        }

        this->restore();
        ch.listener.on_trade_failed(ch, *you, me_message, you_message);
        this->end();
        co_return false;
    }
}
