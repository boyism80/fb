#include <fb/encoding.h>
#include <fb/game/character.h>
#include <fb/game/item.h>
#include <fb/game/map.h>
#include <fb/game/server.h>
#include <fb/logger.h>

#include <json/json.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace fb::model::enum_value;
using namespace fb::game;

using table = fb::model::table;

items::items()
{ }

items::~items()
{ }

void items::owner(std::shared_ptr<character> owner)
{
    this->_owner = owner->weak_from_this_as<character>();
}

std::shared_ptr<character> items::owner() const
{
    return this->_owner.lock();
}

std::shared_ptr<equipment> items::equipment_off(EQUIPMENT_PARTS parts)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    auto equipment = std::shared_ptr<fb::game::equipment>{nullptr};
    switch (parts)
    {
    case EQUIPMENT_PARTS::WEAPON:
        equipment = this->weapon(nullptr);
        break;

    case EQUIPMENT_PARTS::ARMOR:
        equipment = this->armor(nullptr);
        break;

    case EQUIPMENT_PARTS::SHIELD:
        equipment = this->shield(nullptr);
        break;

    case EQUIPMENT_PARTS::HELMET:
        equipment = this->helmet(nullptr);
        break;

    case EQUIPMENT_PARTS::LEFT_HAND:
        equipment = this->ring(nullptr, EQUIPMENT_POSITION::LEFT);
        break;

    case EQUIPMENT_PARTS::RIGHT_HAND:
        equipment = this->ring(nullptr, EQUIPMENT_POSITION::RIGHT);
        break;

    case EQUIPMENT_PARTS::LEFT_AUX:
        equipment = this->auxiliary(nullptr, EQUIPMENT_POSITION::LEFT);
        break;

    case EQUIPMENT_PARTS::RIGHT_AUX:
        equipment = this->auxiliary(nullptr, EQUIPMENT_POSITION::RIGHT);
        break;

    default:
        return nullptr;
    }

    if (equipment == nullptr)
        return nullptr;

    owner->stat.hp(owner->stat.hp(), false);
    owner->stat.mp(owner->stat.mp(), false);
    owner->update(UPDATE_STATE_LEVEL::ALL);

    auto& model = equipment->model();
    auto  path  = std::format("scripts/item/{}.lua", model.id);
    auto  func  = "on_deactivated";

    auto lua = owner->server.lua.open(path, func);
    if (lua)
    {
        lua->pushobject(*owner);
        lua->pushinteger(parts);
        lua->pushobject(*equipment);
        fb::lua::run_async(std::move(lua), 3);
    }

    auto inactive = owner->server.lua.open("scripts/interaction.lua", "on_equipment_inactive");
    if (inactive)
    {
        inactive->pushobject(*owner);
        inactive->pushinteger(parts);
        inactive->pushobject(*equipment);
        fb::lua::run_async(std::move(inactive), 3);
    }

    owner->show();
    return equipment;
}

async::task<uint8_t> items::add(std::shared_ptr<item> item)
{
    auto&& result = co_await this->add(std::vector<std::shared_ptr<fb::game::item>>{item});
    if (result.empty())
        co_return 0xFF;
    else
        co_return result[0];
}

async::task<std::vector<uint8_t>> items::add(const std::vector<std::shared_ptr<item>>& items, bool stop_if_remained)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return std::vector<uint8_t>();

    auto indices = std::vector<uint8_t>();
    auto updates = std::unordered_map<uint8_t, std::shared_ptr<item>>();

    for (auto& item : items)
    {
        if (item == nullptr)
            continue;

        auto death_uid = item->death_uid();
        if (death_uid.has_value() && death_uid.value() != owner->id)
        {
            auto& drop_time = item->dropped_time();
            if (drop_time.has_value())
            {
                auto diff = owner->server.now() - drop_time.value();
                if (diff < fb::model::const_value::death_penalty::warmth_time)
                {
                    owner->message(_TEXT(MESSAGE_ITEM_DEATH_PENALTY_WARMTH));
                    break;
                }
            }
        }

        auto& model = item->model();
        if (model.attr(ITEM_ATTRIBUTE::CASH))
        {
            auto cash   = std::static_pointer_cast<fb::game::cash>(item);
            auto before = cash->value;
            auto remain = owner->money_add(cash->value);
            if (remain != before)
            {
                if (remain > 0)
                {
                    auto new_cash = co_await cash->replace(remain);
                    if (new_cash != nullptr)
                        std::ignore = co_await new_cash->map(owner->map(), owner->position());
                }
                else
                {
                    co_await cash->destroy();
                }
            }

            owner->update(UPDATE_STATE_LEVEL::EXP_MONEY);
            if (remain != 0)
            {
                owner->message(_TEXT(MESSAGE_MONEY_FULL));
                if (stop_if_remained)
                    break;
            }
        }
        else
        {
            auto exists = model.attr(ITEM_ATTRIBUTE::BUNDLE) ? this->find(model) : nullptr;
            if (exists != nullptr)
            {
                auto before = exists->count();
                exists->merge(item);

                auto index = this->index(exists);
                updates.insert({index, exists});
                indices.push_back(index);
                if (exists->count() > before)
                    this->write_item_log("item_gain", *exists, exists->count() - before, index);

                if (item->empty())
                    co_await item->destroy();
                else if (stop_if_remained)
                    break;
            }
            else
            {
                // A bundle whose whole stack is held by an escrow keeps its slot, so the new count joins that slot.
                auto index = uint8_t{0xFF};
                if (model.attr(ITEM_ATTRIBUTE::BUNDLE))
                {
                    for (auto i : this->_escrow.indices())
                    {
                        if (this->_escrow.at(i)->model() == model)
                        {
                            index = i;
                            break;
                        }
                    }
                }

                if (index != 0xFF)
                {
                    auto free_space = static_cast<uint16_t>(model.capacity - this->_escrow.at(index)->count());
                    if (free_space == 0)
                    {
                        owner->message(_TEXT(MESSAGE_ITEM_CANNOT_PICKUP_ANYMORE));
                        if (stop_if_remained)
                            break;
                        continue;
                    }

                    if (item->count() > free_space)
                    {
                        auto part   = item->split(free_space);
                        std::ignore = this->add(part, index);
                        updates.insert({index, part});
                        indices.push_back(index);
                        owner->message(_TEXT(MESSAGE_ITEM_CANNOT_PICKUP_ANYMORE));
                        if (stop_if_remained)
                            break;
                        continue;
                    }
                }
                else
                {
                    index = this->next();
                }

                if (index == 0xFF)
                {
                    owner->message(_TEXT(MESSAGE_ITEM_FULL));
                    break;
                }

                std::ignore = this->add(item, index);

                if (item->_map != nullptr)
                    std::ignore = co_await item->map(nullptr);

                updates.insert({index, item});
                indices.push_back(index);
            }
        }
    }
    co_return indices;
}

uint8_t items::add(std::shared_ptr<item> item, uint8_t index)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0xFF;

    auto held = this->_escrow.at(index);
    if (held != nullptr && (item->model().attr(ITEM_ATTRIBUTE::BUNDLE) == false || held->model() != item->model()))
        return 0xFF;

    if (super::add(item, index) == 0xFF)
        return 0xFF;

    item->container(this);
    item->death_uid(std::nullopt);
    if (item->empty() == false)
    {
        owner->listener.on_item_update(*owner, index);
        this->write_item_log("item_gain", *item, item->count(), index);
    }

    return index;
}

void items::write_item_log(std::string_view                event,
                           const fb::game::item&           item,
                           uint16_t                        count,
                           uint8_t                         index,
                           std::optional<ITEM_DELETE_TYPE> attr) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    auto& model = item.model();
    if (model.attr(ITEM_ATTRIBUTE::CASH))
        return;

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
    log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
    log_data["item_id"]        = static_cast<Json::Int64>(model.id);
    log_data["item_name"]      = UTF8(model.name, PLATFORM::WINDOWS);
    log_data["count"]          = static_cast<Json::Int64>(count);
    log_data["index"]          = index;
    if (attr.has_value())
        log_data["delete_type"] = static_cast<int>(attr.value());
    if (item.uid() != 0)
        log_data["item_uid"] = static_cast<Json::UInt64>(item.uid());
    owner->server.log.write(event, log_data);
}

bool items::store(std::shared_ptr<item> item)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    owner->assert_thread();

    auto& model     = item->model();
    auto  item_id   = model.id;
    auto  item_name = model.name;
    auto  count     = item->count();

    if (model.attr(ITEM_ATTRIBUTE::BUNDLE))
    {
        auto found = std::find_if(this->_stored.begin(),
                                  this->_stored.end(),
                                  [&item](const std::shared_ptr<fb::game::item>& stored) {
                                      auto& model = stored->model();
                                      return item->model() == model;
                                  });

        if (found == this->_stored.end())
        {
            this->_stored.push_back(item);
        }
        else
        {
            auto stored   = *found;
            auto capacity = 0xFFFF - stored->count();
            if (item->count() > capacity)
                return false;

            stored->count(stored->count() + item->count());
        }
    }
    else
    {
        this->_stored.push_back(item);
    }

    item->container(this);

    // Log item deposit event
    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
    log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
    log_data["item_id"]        = static_cast<Json::Int64>(item_id);
    log_data["item_name"]      = UTF8(item_name, PLATFORM::WINDOWS);
    log_data["count"]          = static_cast<Json::Int64>(count);
    if (item->uid() != 0)
        log_data["item_uid"] = static_cast<Json::UInt64>(item->uid());
    owner->server.log.write("item_deposit", log_data);

    return true;
}

async::task<bool> items::store(uint8_t index, uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return false;

    owner->assert_thread();

    auto item = this->at(index);
    if (item == nullptr)
        co_return false;

    if (count == 0 || item->count() < count)
        co_return false;

    auto deleted = this->remove(item, count, ITEM_DELETE_TYPE::NONE, false);
    if (deleted == nullptr)
        co_return false;

    auto result = this->store(deleted);
    if (result == false)
        std::ignore = co_await this->add(deleted);

    co_return result;
}

async::task<bool> items::store(std::string_view name, uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return false;

    owner->assert_thread();

    auto item = this->find(name);
    if (item == nullptr)
        co_return false;

    auto index = this->index(item);
    if (index == 0xFF)
        co_return false;

    co_return co_await this->store(index, count);
}

std::shared_ptr<item> items::stored(const fb::model::item& item) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    owner->assert_thread();

    for (const auto& stored : this->_stored)
    {
        if (stored->model() == item)
            return stored;
    }
    return nullptr;
}

const std::vector<std::shared_ptr<item>>& items::stored() const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        throw std::runtime_error("owner is nullptr");

    owner->assert_thread();

    return this->_stored;
}

async::task<items::item_ptr> items::retrieve(uint8_t index, uint16_t count, bool to_inventory)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return nullptr;

    owner->assert_thread();

    if (count == 0)
        co_return nullptr;

    if (index > this->_stored.size() - 1)
        co_return nullptr;

    auto stored       = this->_stored.at(index);
    auto stored_count = stored->count();
    if (stored_count < count)
        co_return nullptr;

    auto& model     = stored->model();
    auto  item_id   = model.id;
    auto  item_name = model.name;

    auto exists = to_inventory && model.attr(ITEM_ATTRIBUTE::BUNDLE) ? this->find(model) : nullptr;
    if (exists != nullptr)
    {
        if (count > exists->free_space())
            co_return nullptr;

        stored->count(stored_count - count);
        auto added_slot = co_await this->add(stored->model().make(owner->server, count));
        if (stored->empty())
        {
            auto i = this->_stored.begin() + index;
            this->_stored.erase(i);
        }

        auto retrieved = this->at(added_slot);

        // Log item retrieve event
        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
        log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
        log_data["item_id"]        = static_cast<Json::Int64>(item_id);
        log_data["item_name"]      = UTF8(item_name, PLATFORM::WINDOWS);
        log_data["count"]          = static_cast<Json::Int64>(count);
        owner->server.log.write("item_retrieve", log_data);

        co_return retrieved;
    }
    else
    {
        if (to_inventory)
        {
            // A bundle whose whole stack is listed joins the listed slot instead of taking a new one.
            auto space = model.attr(ITEM_ATTRIBUTE::BUNDLE) ? this->free_space(model) : model.capacity;
            if (space == model.capacity && this->free() == false)
                co_return nullptr;

            if (count > space)
                co_return nullptr;
        }

        auto item = stored->split(count);
        if (item == stored)
            this->_stored.erase(this->_stored.begin() + index);

        if (to_inventory)
            std::ignore = co_await this->add(item);

        // Log item retrieve event
        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
        log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
        log_data["item_id"]        = static_cast<Json::Int64>(item_id);
        log_data["item_name"]      = UTF8(item_name, PLATFORM::WINDOWS);
        log_data["count"]          = static_cast<Json::Int64>(count);
        if (item->uid() != 0)
            log_data["item_uid"] = static_cast<Json::UInt64>(item->uid());
        owner->server.log.write("item_retrieve", log_data);

        co_return item;
    }
}

async::task<items::item_ptr> items::retrieve(std::string_view name, uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return nullptr;

    owner->assert_thread();

    for (size_t i = 0; i < this->_stored.size(); ++i)
    {
        auto& model = this->_stored[i]->model();
        if (model.name == name)
            co_return co_await this->retrieve(static_cast<uint8_t>(i), count);
    }
    co_return nullptr;
}

async::task<items::item_ptr> items::retrieve(const fb::model::item& item, uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return nullptr;

    owner->assert_thread();

    for (size_t i = 0; i < this->_stored.size(); ++i)
    {
        auto& model = this->_stored[i]->model();
        if (model == item)
            co_return co_await this->retrieve(static_cast<uint8_t>(i), count);
    }
    co_return nullptr;
}

uint64_t items::deposited() const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0;

    owner->assert_thread();

    return this->_deposited;
}

void items::deposited(uint64_t value)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    owner->assert_thread();

    this->_deposited = value;
}

items::money_transfer items::deposit(uint64_t value)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return money_transfer::LACK_MONEY;

    owner->assert_thread();

    if (owner->money() < value)
        return money_transfer::LACK_MONEY;

    auto capacity = this->_deposited < MONEY_LIMIT ? MONEY_LIMIT - this->_deposited : 0;
    if (value > capacity)
        return money_transfer::DEPOSIT_FULL;

    if (value == 0)
        return money_transfer::DONE;

    owner->money(owner->money() - value);
    this->_deposited += value;

    auto log_data                 = Json::Value();
    log_data["character_id"]      = static_cast<Json::Int64>(owner->id);
    log_data["character_name"]    = UTF8(owner->name(), PLATFORM::WINDOWS);
    log_data["amount"]            = static_cast<Json::Int64>(value);
    log_data["current_deposited"] = static_cast<Json::Int64>(this->_deposited);
    log_data["current_money"]     = static_cast<Json::Int64>(owner->money());
    owner->server.log.write("money_deposit", log_data);
    return money_transfer::DONE;
}

items::money_transfer items::withdraw(uint64_t value)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return money_transfer::LACK_DEPOSITED;

    owner->assert_thread();

    if (this->_deposited < value)
        return money_transfer::LACK_DEPOSITED;

    auto held       = owner->money() + this->_escrow.money();
    auto free_space = held < MONEY_LIMIT ? MONEY_LIMIT - held : 0;
    if (value > free_space)
        return money_transfer::MONEY_FULL;

    if (value == 0)
        return money_transfer::DONE;

    this->_deposited -= value;
    owner->money(owner->money() + value);

    auto log_data                 = Json::Value();
    log_data["character_id"]      = static_cast<Json::Int64>(owner->id);
    log_data["character_name"]    = UTF8(owner->name(), PLATFORM::WINDOWS);
    log_data["amount"]            = static_cast<Json::Int64>(value);
    log_data["current_deposited"] = static_cast<Json::Int64>(this->_deposited);
    log_data["current_money"]     = static_cast<Json::Int64>(owner->money());
    owner->server.log.write("money_withdraw", log_data);
    return money_transfer::DONE;
}

async::task<items::item_ptr> items::active(uint8_t index)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return nullptr;

    try
    {
        owner->assert_state({STATE::RIDING, STATE::GHOST});

        auto item = this->at(index);
        if (item == nullptr)
            co_return nullptr;

        auto weak   = item->weak_from_this_as<fb::game::item>();
        std::ignore = co_await item->active();
        if (weak.lock() == nullptr)
            co_return nullptr;

        if (item->empty())
        {
            co_await item->destroy();
            co_return nullptr;
        }
        else
        {
            co_return item;
        }
    }
    catch (std::exception& e)
    {
        owner->message(e.what());
    }
    co_return nullptr;
}

async::task<uint8_t> items::inactive(EQUIPMENT_PARTS parts)
{
    if (this->free() == false)
        co_return 0xFF;

    auto item = this->equipment_off(parts);
    if (item == nullptr)
        co_return 0xFF;

    auto slot = co_await this->add(item);
    co_return slot;
}

uint8_t items::index(const fb::model::item& model) const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto now = this->at(i);

        if (now == nullptr)
            continue;

        if (now->model() == model)
            return i;
    }

    return 0xFF;
}

uint8_t items::index(const std::shared_ptr<item>& item) const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto now = this->at(i);
        if (now == item)
            return i;
    }

    return 0xFF;
}

std::vector<uint8_t> items::index_all(const std::shared_ptr<item>& item) const
{
    auto result = std::vector<uint8_t>();
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto now = this->at(i);

        if (now == nullptr)
            continue;

        if (now->model() == item->model())
            result.push_back(i);
    }

    return result;
}

bool items::update(uint8_t index) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    auto item = this->at(index);
    if (item == nullptr)
        return false;

    auto remained = item->count() - item->trade_count();
    if (remained == 0)
        owner->listener.on_item_remove(*owner, index, ITEM_DELETE_TYPE::NONE);
    else
        owner->listener.on_item_update(*owner, index);
    return true;
}

std::shared_ptr<equipment> items::wear(EQUIPMENT_PARTS parts, equipment_ptr item)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    if (item != nullptr)
        item->container(this);

    switch (parts) // EQUIPMENT_PARTS
    {
    case EQUIPMENT_PARTS::WEAPON:
        return owner->items.weapon(std::static_pointer_cast<fb::game::weapon>(item));

    case EQUIPMENT_PARTS::ARMOR:
        return owner->items.armor(std::static_pointer_cast<fb::game::armor>(item));

    case EQUIPMENT_PARTS::SHIELD:
        return owner->items.shield(std::static_pointer_cast<fb::game::shield>(item));

    case EQUIPMENT_PARTS::HELMET:
        return owner->items.helmet(std::static_pointer_cast<fb::game::helmet>(item));

    case EQUIPMENT_PARTS::LEFT_HAND:
        return owner->items.ring(std::static_pointer_cast<fb::game::ring>(item), EQUIPMENT_POSITION::LEFT);

    case EQUIPMENT_PARTS::RIGHT_HAND:
        return owner->items.ring(std::static_pointer_cast<fb::game::ring>(item), EQUIPMENT_POSITION::RIGHT);

    case EQUIPMENT_PARTS::LEFT_AUX:
        return owner->items.auxiliary(std::static_pointer_cast<fb::game::auxiliary>(item), EQUIPMENT_POSITION::LEFT);

    case EQUIPMENT_PARTS::RIGHT_AUX:
        return owner->items.auxiliary(std::static_pointer_cast<fb::game::auxiliary>(item), EQUIPMENT_POSITION::RIGHT);

    default:
        throw std::runtime_error("invalid equipment parts");
    }
}

// Nothing is sent here; sync() sends the restored state once the client has the map.
void items::load(std::shared_ptr<item> item, const fb::protocol::internal::Item& row)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    item->container(this);
    if (row.stored == ESCROW_STORED)
    {
        auto regular = row.index >= 0 && row.index < CONTAINER_CAPACITY ? this->at(row.index) : nullptr;
        if (row.index < 0 || row.index >= CONTAINER_CAPACITY || this->_escrow.reserved(row.index) ||
            (regular != nullptr &&
             (regular->model().attr(ITEM_ATTRIBUTE::BUNDLE) == false || regular->model() != item->model())))
        {
            fb::logger::fatal("{} escrow item {} cannot be restored at slot {} (escrow {})",
                              owner->name(),
                              item->model().id,
                              row.index,
                              row.escrow_id.value_or(0));
            return;
        }

        std::ignore = this->_escrow.restore(row.escrow_id.value_or(0), static_cast<uint8_t>(row.index), item);
    }
    else if (row.stored != -1)
    {
        // Storage stacks may exceed the model capacity, which the item constructor clamps to.
        if (row.count > item->count())
            item->count(row.count);
        this->_stored.push_back(item);
    }
    else if (row.parts == static_cast<int16_t>(EQUIPMENT_PARTS::UNKNOWN))
    {
        auto held = row.index >= 0 && row.index < CONTAINER_CAPACITY ? this->_escrow.at(row.index) : nullptr;
        if (row.index < 0 || row.index >= CONTAINER_CAPACITY ||
            (held != nullptr &&
             (item->model().attr(ITEM_ATTRIBUTE::BUNDLE) == false || held->model() != item->model())) ||
            super::add(item, row.index) == 0xFF)
        {
            fb::logger::warn("{} item {} cannot be restored at slot {}; moved to storage",
                             owner->name(),
                             item->model().id,
                             row.index);
            this->_stored.push_back(item);
        }
    }
    else
    {
        auto parts    = static_cast<EQUIPMENT_PARTS>(row.parts);
        auto occupied = false;
        switch (parts)
        {
        case EQUIPMENT_PARTS::WEAPON:
            occupied = this->_weapon != nullptr;
            if (occupied == false)
                this->_weapon = std::static_pointer_cast<fb::game::weapon>(item);
            break;

        case EQUIPMENT_PARTS::ARMOR:
            occupied = this->_armor != nullptr;
            if (occupied == false)
                this->_armor = std::static_pointer_cast<fb::game::armor>(item);
            break;

        case EQUIPMENT_PARTS::SHIELD:
            occupied = this->_shield != nullptr;
            if (occupied == false)
                this->_shield = std::static_pointer_cast<fb::game::shield>(item);
            break;

        case EQUIPMENT_PARTS::HELMET:
            occupied = this->_helmet != nullptr;
            if (occupied == false)
                this->_helmet = std::static_pointer_cast<fb::game::helmet>(item);
            break;

        case EQUIPMENT_PARTS::LEFT_HAND:
        case EQUIPMENT_PARTS::RIGHT_HAND:
        {
            auto& ring = this->_rings[static_cast<int>(
                parts == EQUIPMENT_PARTS::LEFT_HAND ? EQUIPMENT_POSITION::LEFT : EQUIPMENT_POSITION::RIGHT)];
            occupied   = ring != nullptr;
            if (occupied == false)
                ring = std::static_pointer_cast<fb::game::ring>(item);
        }
        break;

        case EQUIPMENT_PARTS::LEFT_AUX:
        case EQUIPMENT_PARTS::RIGHT_AUX:
        {
            auto& auxiliary = this->_auxiliaries[static_cast<int>(
                parts == EQUIPMENT_PARTS::LEFT_AUX ? EQUIPMENT_POSITION::LEFT : EQUIPMENT_POSITION::RIGHT)];
            occupied        = auxiliary != nullptr;
            if (occupied == false)
                auxiliary = std::static_pointer_cast<fb::game::auxiliary>(item);
        }
        break;

        default:
            occupied = true;
            break;
        }

        if (occupied)
        {
            fb::logger::warn("{} equipment {} cannot be restored at parts {}; moved to storage",
                             owner->name(),
                             item->model().id,
                             row.parts);
            this->_stored.push_back(item);
        }
        else
        {
            owner->stat.equipment_on(std::static_pointer_cast<fb::game::equipment>(item)->model());
        }
    }
}

void items::sync() const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    for (uint8_t i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if (this->at(i) != nullptr || this->_escrow.reserved(i))
            owner->listener.on_item_update(*owner, i);
    }

    auto equipments = this->equipments();
    if (std::ranges::any_of(equipments, [](const auto& x) {
            return x.second != nullptr;
        }))
    {
        owner->listener.on_equipment_sync(*owner);
        owner->update_external();
    }
}

void items::notify_equipment_swap(EQUIPMENT_PARTS parts, const equipment_ptr& before, const equipment_ptr& after)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    if (before == after)
        return;

    if (before != nullptr)
    {
        owner->stat.equipment_off(before->model());
        owner->listener.on_equipment_off(*owner, parts, *before);
    }

    if (after != nullptr)
    {
        owner->stat.equipment_on(after->model());
        owner->listener.on_equipment_on(*owner, *after, parts);
    }
}

std::shared_ptr<weapon> items::weapon() const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    return this->_weapon;
}

std::shared_ptr<weapon> items::weapon(std::shared_ptr<fb::game::weapon> weapon)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    auto before = this->_weapon;

    this->_weapon = weapon;
    this->notify_equipment_swap(EQUIPMENT_PARTS::WEAPON, before, weapon);
    owner->update_external();
    return before;
}

std::shared_ptr<armor> items::armor() const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    return this->_armor;
}

std::shared_ptr<armor> items::armor(std::shared_ptr<fb::game::armor> armor)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    auto before = this->_armor;

    this->_armor = armor;
    this->notify_equipment_swap(EQUIPMENT_PARTS::ARMOR, before, armor);
    owner->update_external();

    return before;
}

std::shared_ptr<shield> items::shield() const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    return this->_shield;
}

std::shared_ptr<shield> items::shield(std::shared_ptr<fb::game::shield> shield)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    auto before = this->_shield;

    this->_shield = shield;
    this->notify_equipment_swap(EQUIPMENT_PARTS::SHIELD, before, shield);
    owner->update_external();

    return before;
}

std::shared_ptr<helmet> items::helmet() const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    return this->_helmet;
}

std::shared_ptr<helmet> items::helmet(std::shared_ptr<fb::game::helmet> helmet)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    auto before = this->_helmet;

    this->_helmet = helmet;
    this->notify_equipment_swap(EQUIPMENT_PARTS::HELMET, before, helmet);
    owner->update_external();
    if (owner->option(OPTION::VISIBLE_HELMET))
        owner->group.refresh_portrait();

    return before;
}

std::shared_ptr<ring> items::ring(EQUIPMENT_POSITION position) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    return this->_rings[static_cast<int>(position)];
}

std::shared_ptr<ring> items::ring(std::shared_ptr<fb::game::ring> ring)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    std::shared_ptr<fb::game::ring> before = nullptr;

    if (this->_rings[static_cast<int>(EQUIPMENT_POSITION::LEFT)] == nullptr)
    {
        before = this->ring(ring, EQUIPMENT_POSITION::LEFT);
    }
    else
    {
        before = this->ring(ring, EQUIPMENT_POSITION::RIGHT);
    }
    return before;
}

std::shared_ptr<ring> items::ring(std::shared_ptr<fb::game::ring> ring, EQUIPMENT_POSITION position)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    auto before = this->_rings[static_cast<int>(position)];

    this->_rings[static_cast<int>(position)] = ring;
    auto parts = (position == EQUIPMENT_POSITION::LEFT) ? EQUIPMENT_PARTS::LEFT_HAND : EQUIPMENT_PARTS::RIGHT_HAND;
    this->notify_equipment_swap(parts, before, ring);
    owner->update_external();

    return before;
}

std::shared_ptr<auxiliary> items::auxiliary(EQUIPMENT_POSITION position) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    return this->_auxiliaries[static_cast<int>(position)];
}

std::shared_ptr<auxiliary> items::auxiliary(std::shared_ptr<fb::game::auxiliary> auxiliary)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    std::shared_ptr<fb::game::auxiliary> before = nullptr;

    if (this->_auxiliaries[static_cast<int>(EQUIPMENT_POSITION::LEFT)] == nullptr)
    {
        before = this->auxiliary(auxiliary, EQUIPMENT_POSITION::LEFT);
    }
    else
    {
        before = this->auxiliary(auxiliary, EQUIPMENT_POSITION::RIGHT);
    }

    return before;
}

std::shared_ptr<auxiliary> items::auxiliary(std::shared_ptr<fb::game::auxiliary> auxiliary, EQUIPMENT_POSITION position)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    auto before                                    = this->_auxiliaries[static_cast<int>(position)];
    this->_auxiliaries[static_cast<int>(position)] = auxiliary;
    auto parts = (position == EQUIPMENT_POSITION::LEFT) ? EQUIPMENT_PARTS::LEFT_AUX : EQUIPMENT_PARTS::RIGHT_AUX;
    this->notify_equipment_swap(parts, before, auxiliary);
    owner->update_external();

    return before;
}

std::shared_ptr<item> items::find(std::string_view name) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->at(i);
        if (item == nullptr)
            continue;

        auto& model = item->model();
        if (model.name == name)
            return std::static_pointer_cast<fb::game::item>(item);
    }

    return nullptr;
}

std::shared_ptr<item> items::find(const fb::model::item& model) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->at(i);
        if (item == nullptr)
            continue;

        if (item->model() == model)
            return std::static_pointer_cast<fb::game::item>(item);
    }

    for (auto& [parts, equipment] : this->equipments())
    {
        if (equipment == nullptr)
            continue;

        if (equipment->model() == model)
            return std::static_pointer_cast<fb::game::item>(equipment);
    }

    return nullptr;
}

std::shared_ptr<item> items::find(uint64_t uid) const
{
    if (uid == 0)
        return nullptr;

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->at(i);
        if (item == nullptr)
            continue;

        if (item->uid() == uid)
            return std::static_pointer_cast<fb::game::item>(item);
    }

    for (auto& [parts, equipment] : this->equipments())
    {
        if (equipment == nullptr)
            continue;

        if (equipment->uid() == uid)
            return std::static_pointer_cast<fb::game::item>(equipment);
    }

    return nullptr;
}

uint16_t items::free_space(const fb::model::item& model) const
{
    auto item = this->find(model);
    if (item != nullptr)
        return item->free_space();

    for (auto i : this->_escrow.indices())
    {
        auto held = this->_escrow.at(i);
        if (held->model() == model)
            return model.capacity - held->count();
    }

    return model.capacity;
}

bool items::has(const fb::model::item& model, uint16_t count) const
{
    if (model.attr(ITEM_ATTRIBUTE::BUNDLE))
    {
        /* BUNDLE stacks in a single slot; at most one slot holds this item. */
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            auto slot_item = this->at(i);
            if (slot_item == nullptr)
                continue;
            if (slot_item->model() == model)
                return slot_item->count() >= count;
        }
        return false;
    }
    else
    {
        /* Non-BUNDLE: multiple slots possible, capacity 1 each; count slots. */
        uint32_t slot_count = 0;
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            auto slot_item = this->at(i);
            if (slot_item != nullptr && slot_item->model() == model)
                slot_count++;
        }
        return slot_count >= count;
    }
}

bool items::has(const std::vector<std::pair<const fb::model::item*, uint16_t>>& required) const
{
    if (required.empty())
        return true;

    std::unordered_map<uint32_t, uint32_t> counts;
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto slot_item = this->at(i);
        if (slot_item == nullptr)
            continue;
        const auto& model = slot_item->model();
        if (model.attr(ITEM_ATTRIBUTE::BUNDLE))
            counts[model.id] += slot_item->count();
        else
            counts[model.id] += 1;
    }

    for (const auto& [model, min_count] : required)
    {
        if (model == nullptr)
            return false;
        auto it = counts.find(model->id);
        if (it == counts.end() || it->second < min_count)
            return false;
    }
    return true;
}

async::task<items::item_ptr> items::drop(uint8_t index, uint8_t count, bool action, ITEM_DELETE_TYPE delete_type)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return nullptr;

    try
    {
        if (action)
            owner->assert_state({STATE::RIDING, STATE::GHOST});

        auto item = this->at(index);
        if (item == nullptr)
            co_return nullptr;

        auto& model = item->model();
        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_CANNOT_DROP_ITEM));

        auto dropped = this->remove(item, count, delete_type);
        if (dropped != nullptr)
        {
            std::ignore = co_await dropped->map(owner->map(), owner->position());

            if (action)
                owner->action(ACTION::PICKUP, DURATION::PICKUP);
        }

        co_return dropped;
    }
    catch (std::exception& e)
    {
        owner->message(e.what());
    }
    co_return nullptr;
}

async::task<void> items::loot(bool boost)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return;

    try
    {
        auto map = owner->map();
        if (map == nullptr)
            co_return;

        auto lua = owner->server.lua.open("scripts/interaction.lua", "on_loot");
        if (lua)
        {
            lua->pushobject(*owner);
            fb::lua::run_async(std::move(lua), 1);
        }

        owner->assert_state({STATE::GHOST, STATE::RIDING});
        owner->action(ACTION::PICKUP, DURATION::PICKUP);

        // Pick up items in reverse order
        auto belows = std::vector<std::shared_ptr<item>>();
        for (auto& below : map->belows(owner->position(), OBJECT_TYPE::ITEM))
        {
            belows.push_back(std::static_pointer_cast<item>(below));
        }

        if (belows.size() == 0)
        {
        }
        else if (boost)
        {
            std::ignore = co_await owner->items.add(belows, true);
        }
        else
        {
            std::ignore = co_await owner->items.add(belows.front());
        }
    }
    catch (std::exception& e)
    {
        owner->message(e.what());
    }
}

async::task<bool> items::throws(uint8_t index, bool all)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return false;

    try
    {
        auto item = owner->items.at(index);
        if (item == nullptr)
            co_return false;

        auto& model = item->model();
        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_CANNOT_THROW_ITEM));

        auto map = owner->map();
        if (map == nullptr)
            throw std::exception();

        auto count    = all ? item->count() : 1;
        auto dropped  = this->remove(index, count, ITEM_DELETE_TYPE::THROW);
        auto position = owner->position();
        for (int i = 0; i < 7; i++)
        {
            auto before = position;
            position.forward(owner->direction());
            if (map->movable(*owner, position) == false)
            {
                position = before;
                break;
            }
        }

        owner->listener.on_item_throws(*owner, *dropped, position);
        std::ignore = co_await dropped->map(map, position);
        co_return true;
    }
    catch (std::exception& e)
    {
        owner->message(e.what());
    }
    co_return false;
}

std::shared_ptr<item> items::remove(uint8_t index, uint16_t count, ITEM_DELETE_TYPE attr, bool detach)
{
    // split(0) on a bundle returns a new zero-count item and leaves the stack intact.
    if (count == 0)
        return nullptr;

    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    auto item = this->at(index);
    if (item == nullptr)
        return nullptr;

    auto splitted = item->split(count);
    // The partner agreed to the offered count, so an offer that can no longer be covered ends the trade.
    auto short_offer = splitted == item ? item->trade_count() > 0 : item->count() < item->trade_count();
    if (splitted == item)
    {
        if (detach)
            splitted->_container = nullptr;

        splitted->trade_count(0);

        std::ignore = inventory<fb::game::item>::remove(index);
        owner->listener.on_item_remove(*owner, index, attr);
    }
    else
    {
        item->trade_count(std::min(item->trade_count(), item->count()));
    }
    this->write_item_log("item_remove", *splitted, splitted->count(), index, attr);

    if (short_offer && owner->trade != nullptr)
    {
        auto trade = owner->trade;
        trade->cancel(*owner);
    }

    auto current = this->at(index);
    owner->listener.on_item_update(*owner, index);
    return splitted;
}

std::shared_ptr<item> items::remove(std::shared_ptr<item> item, uint16_t count, ITEM_DELETE_TYPE attr, bool detach)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    auto index = this->index(item);
    if (index == 0xFF)
        return nullptr;

    return this->remove(index, count, attr, detach);
}

async::task<void> items::remove_expired()
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return;

    owner->assert_thread();

    auto expired_items = std::vector<std::shared_ptr<item>>();
    for (int i = 0; i < CONTAINER_CAPACITY;)
    {
        auto item = this->at(i);
        if (item != nullptr && item->expired())
        {
            auto expired = this->remove(i, item->count(), ITEM_DELETE_TYPE::NONE);
            if (expired == nullptr)
                continue;

            owner->message(std::format("{} 아이템이 만료되었습니다.", expired->model().name));
            co_await expired->destroy();
            continue;
        }

        ++i;
    }

    for (auto& [parts, equipment] : this->equipments())
    {
        if (equipment != nullptr && equipment->expired())
        {
            auto expired = this->equipment_off(parts);
            if (expired == nullptr)
                continue;

            expired_items.push_back(expired);
        }
    }

    for (int i = static_cast<int>(this->_stored.size()) - 1; i >= 0; --i)
    {
        auto stored = this->_stored.at(i);
        if (stored == nullptr)
            continue;

        if (stored->expired() == false)
            continue;

        expired_items.push_back(stored);
        this->_stored.erase(this->_stored.begin() + i);
    }

    for (auto& expired : expired_items)
    {
        if (expired == nullptr)
            continue;

        auto& model = expired->model();
        owner->message(std::format("{} 아이템이 만료되었습니다.", model.name));

        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
        log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
        log_data["item_id"]        = static_cast<Json::Int64>(model.id);
        log_data["item_name"]      = UTF8(model.name, PLATFORM::WINDOWS);
        log_data["count"]          = static_cast<Json::Int64>(expired->count());
        if (expired->uid() != 0)
            log_data["item_uid"] = static_cast<Json::UInt64>(expired->uid());
        owner->server.log.write("item_remove", log_data);

        expired->container(nullptr);
        co_await expired->destroy();
    }
}

bool items::swap(uint8_t src, uint8_t dst)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    if (inventory<item>::swap(src, dst) == false)
        return false;

    // The client sees a slot as one stack, so a held part moves with the slot.
    this->_escrow.swap(src, dst);

    const auto right = this->at(src);
    if (right != nullptr || this->reserved(src))
        owner->listener.on_item_update(*owner, src);
    else
        owner->listener.on_item_remove(*owner, src);

    const auto left = this->at(dst);
    if (left != nullptr || this->reserved(dst))
        owner->listener.on_item_update(*owner, dst);
    else
        owner->listener.on_item_remove(*owner, dst);

    owner->listener.on_item_swap(*owner, src, dst);
    return true;
}

bool items::is_rewardable(const std::unordered_map<uint32_t, uint16_t>& items, uint64_t money) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    auto money_cap = std::numeric_limits<uint64_t>::max() - owner->money() - this->_escrow.money();
    if (money_cap < money)
        return false;

    auto required_size = 0;
    for (auto& [id, count] : items)
    {
        auto  item_table = table::item;
        auto& model      = item_table[id];
        if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
        {
            required_size += count;
        }
        else
        {
            // A bundle never takes a second slot, so more than one stack cannot be received at all.
            if (count > model.capacity)
                return false;

            required_size++;
        }
    }

    auto free_size = this->free_size();
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item  = this->at(i);
        auto held  = this->_escrow.at(i);
        auto shown = item != nullptr ? item : held;
        if (shown == nullptr)
            continue;

        auto& model = shown->model();
        if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
            continue;

        if (items.contains(model.id) == false)
            continue;

        auto count = (item != nullptr ? item->count() : 0) + (held != nullptr ? held->count() : 0);
        if (model.capacity < count + items.at(model.id))
            return false;

        free_size++;
    }

    return free_size >= required_size;
}

bool items::is_rewardable(const std::vector<fb::model::dsl>& items) const
{
    auto buffer = std::unordered_map<uint32_t, uint16_t>{};
    auto money  = uint64_t{0};
    for (auto& item : items)
    {
        switch (item.header)
        {
        case fb::model::enum_value::DSL::item:
        {
            auto params        = fb::model::dsl::item(item.params);
            buffer[params.id] += params.count;
            break;
        }
        case fb::model::enum_value::DSL::money:
        {
            auto params  = fb::model::dsl::money(item.params);
            money       += params.value;
            break;
        }
        default:
            break;
        }
    }
    return this->is_rewardable(buffer, money);
}

async::task<exchange_result> items::exchange(const std::unordered_map<uint32_t, uint16_t>& cost_items,
                                             uint64_t                                      cost_money,
                                             const std::unordered_map<uint32_t, uint16_t>& reward_items,
                                             uint64_t                                      reward_money,
                                             ITEM_DELETE_TYPE                              delete_type)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return exchange_result::lack_cost;

    if (owner->money() < cost_money)
        co_return exchange_result::lack_cost;

    auto effective_free_slots       = this->free_size();
    auto simulated_bundle_remaining = std::unordered_map<uint32_t, uint16_t>{};
    auto slots_by_id                = std::unordered_map<uint32_t, std::vector<uint8_t>>{};

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto slot = this->at(static_cast<uint8_t>(i));
        if (slot == nullptr)
            continue;
        auto& model = slot->model();
        slots_by_id[model.id].push_back(static_cast<uint8_t>(i));
    }

    for (auto& [id, cost_count] : cost_items)
    {
        auto  item_table2 = table::item;
        auto& model       = item_table2[id];
        auto  it          = slots_by_id.find(id);
        if (it == slots_by_id.end() || it->second.empty())
            co_return exchange_result::lack_cost;

        if (model.attr(ITEM_ATTRIBUTE::BUNDLE))
        {
            auto index = it->second[0];
            auto slot  = this->at(index);
            if (slot == nullptr)
                co_return exchange_result::lack_cost;

            auto current = slot->count();
            if (current < cost_count)
                co_return exchange_result::lack_cost;

            auto remain = static_cast<uint16_t>(current - cost_count);
            auto locked = this->locked_count(*slot);
            if (remain == 0 && locked == 0)
                effective_free_slots++;
            else
                simulated_bundle_remaining[id] = remain + locked;
        }
        else
        {
            auto slot_count = static_cast<uint32_t>(it->second.size());
            if (slot_count < cost_count)
                co_return exchange_result::lack_cost;

            effective_free_slots =
                static_cast<uint8_t>(std::min(0xFF, static_cast<int>(effective_free_slots) + cost_count));
        }
    }

    uint64_t money_after         = owner->money() + this->_escrow.money() - cost_money;
    uint64_t effective_money_cap = std::numeric_limits<uint64_t>::max() - money_after;
    if (effective_money_cap < reward_money)
        co_return exchange_result::lack_capacity;

    int required_size = 0;
    for (auto& [id, count] : reward_items)
    {
        auto  item_table3  = table::item;
        auto& model        = item_table3[id];
        auto  capacity     = std::max<uint32_t>(model.capacity, 1);
        required_size     += static_cast<int>((count + capacity - 1) / capacity);
    }

    int available_slots = effective_free_slots;
    for (auto& [id, count] : reward_items)
    {
        auto  item_table4 = table::item;
        auto& model       = item_table4[id];
        if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
            continue;

        auto existing = std::optional<uint32_t>{};
        if (cost_items.contains(id))
        {
            auto it = simulated_bundle_remaining.find(id);
            if (it != simulated_bundle_remaining.end())
                existing = it->second;
        }
        else
        {
            auto space = this->free_space(model);
            if (space < model.capacity)
                existing = static_cast<uint32_t>(model.capacity - space);
        }

        // Without a stack to merge into, the bundle needs a free slot and must fit in one stack.
        if (existing.has_value() == false)
        {
            if (model.capacity < count)
                co_return exchange_result::lack_capacity;

            continue;
        }

        if (model.capacity < existing.value() + count)
            co_return exchange_result::lack_capacity;

        available_slots++;
    }

    if (available_slots < required_size)
        co_return exchange_result::lack_capacity;

    owner->money_reduce(cost_money);

    for (auto& [id, cost_count] : cost_items)
    {
        auto&    slots     = slots_by_id[id];
        size_t   slot_it   = 0;
        uint16_t remaining = cost_count;
        while (remaining > 0 && slot_it < slots.size())
        {
            auto index = slots[slot_it];
            auto slot  = this->at(index);
            if (slot == nullptr || slot->model().id != id)
            {
                ++slot_it;
                continue;
            }

            auto in_slot = slot->count();
            if (in_slot == 0)
            {
                ++slot_it;
                continue;
            }

            uint16_t to_remove = std::min(in_slot, remaining);
            auto     removed   = this->remove(index, to_remove, delete_type, true);
            if (removed != nullptr)
                co_await removed->destroy();
            remaining -= to_remove;

            if (slot->count() == 0)
                ++slot_it;
        }
    }

    owner->money_add(reward_money);

    for (auto& [id, count] : reward_items)
    {
        auto  item_table5 = table::item;
        auto& model       = item_table5[id];
        auto  capacity    = std::max<uint32_t>(model.capacity, 1);
        auto  remaining   = static_cast<uint32_t>(count);
        // The item constructor clamps the count to capacity, so a larger reward is made in pieces.
        while (remaining > 0)
        {
            auto piece  = std::min(remaining, capacity);
            remaining  -= piece;

            auto made = model.make(owner->server, piece);
            if (made == nullptr)
                continue;

            auto idx = co_await this->add(made);
            if (idx == 0xFF)
                co_await made->destroy();
        }
    }

    co_return exchange_result::ok;
}

async::task<bool> items::combine(const std::vector<uint8_t>& indices)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        co_return false;

    // The recipe is matched against the selected slots only, so materials are consumed from those slots only.
    // A repeated index would count the same stack twice.
    auto selected = std::vector<uint8_t>();
    auto dsl      = std::vector<fb::model::dsl::item>();
    for (auto index : indices)
    {
        if (std::find(selected.begin(), selected.end(), index) != selected.end())
            continue;

        auto item = this->at(index);
        if (item == nullptr)
            continue;

        selected.push_back(index);
        dsl.push_back(
            fb::model::dsl::item(item->model().id, item->count(), std::nullopt, std::nullopt, 100.0, std::nullopt));
    }

    auto found = table::recipe->find(dsl);
    if (found == nullptr)
    {
        owner->message(_TEXT(MESSAGE_NO_RECIPE));
        co_return false;
    }

    // The consumption is planned first so that the space check and the actual removal use the same numbers.
    auto consume = std::map<uint8_t, uint16_t>();
    for (auto& x : found->source)
    {
        auto params    = fb::model::dsl::item(x.params);
        auto remaining = static_cast<uint32_t>(params.count);
        for (auto index : selected)
        {
            if (remaining == 0)
                break;

            auto item = this->at(index);
            if (item == nullptr || item->model().id != params.id)
                continue;

            auto count      = std::min<uint32_t>(item->count() - consume[index], remaining);
            consume[index] += static_cast<uint16_t>(count);
            remaining      -= count;
        }

        if (remaining > 0)
        {
            owner->message(_TEXT(MESSAGE_NO_RECIPE));
            co_return false;
        }
    }

    // A material stack that keeps some count after consumption still occupies its slot.
    auto free_slots = static_cast<int>(this->free_size());
    for (auto& [index, count] : consume)
    {
        if (this->at(index)->count() == count && this->reserved(index) == false)
            free_slots++;
    }

    // items::add merges a bundle into its existing stack and drops what does not fit, so a bundle result must fit
    // in one stack: the remaining material stack of the same model, the slot whose whole stack is listed, or a new
    // slot.
    auto fits = [&](const auto& results) {
        auto totals = std::map<uint32_t, uint32_t>();
        for (auto& x : results)
        {
            auto params        = fb::model::dsl::item(x.params);
            totals[params.id] += params.count;
        }

        auto required = 0;
        for (auto& [id, total] : totals)
        {
            auto  item_table = table::item;
            auto& model      = item_table[id];
            if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
            {
                required += static_cast<int>(total);
                continue;
            }

            auto existing = this->find(model);
            auto left     = uint32_t{0};
            if (existing != nullptr)
            {
                auto index = this->index(existing);
                left =
                    existing->count() - (consume.contains(index) ? consume[index] : 0) + this->locked_count(*existing);
            }
            else
            {
                left = model.capacity - this->free_space(model);
            }

            if (left + total > model.capacity)
                return false;

            if (left == 0)
                required++;
        }
        return required <= free_slots;
    };

    if (fits(found->success) == false || fits(found->failed) == false)
    {
        owner->message(_TEXT(MESSAGE_EXCEPTION_INVENTORY_OVERFLOW));
        co_return false;
    }

    // Source items are consumed regardless of success or failure.
    for (auto& [index, count] : consume)
    {
        auto removed = this->remove(index, count);
        if (removed != nullptr)
            co_await removed->destroy();
    }

    auto  success = (std::rand() % 100) < found->percent;
    auto& result  = success ? found->success : found->failed;
    for (auto& x : result)
    {
        auto  params     = fb::model::dsl::item(x.params);
        auto  item_table = table::item;
        auto& model      = item_table[params.id];
        auto  remain     = params.count;
        while (remain > 0)
        {
            auto count   = std::min<uint16_t>(model.capacity, remain);
            auto made    = model.make(owner->server, count);
            std::ignore  = co_await this->add(made);
            remain      -= count;
        }
    }

    owner->message(success ? _TEXT(MESSAGE_MIX_SUCCESS) : _TEXT(MESSAGE_MIX_FAILED));

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
    log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
    log_data["success"]        = success;
    auto source_items          = Json::Value(Json::arrayValue);
    for (auto& x : found->source)
    {
        auto params          = fb::model::dsl::item(x.params);
        auto item_data       = Json::Value();
        item_data["item_id"] = static_cast<Json::Int64>(params.id);
        item_data["count"]   = static_cast<Json::Int64>(params.count);
        source_items.append(item_data);
    }
    log_data["source_items"] = source_items;
    auto result_items        = Json::Value(Json::arrayValue);
    for (auto& x : result)
    {
        auto params          = fb::model::dsl::item(x.params);
        auto item_data       = Json::Value();
        item_data["item_id"] = static_cast<Json::Int64>(params.id);
        item_data["count"]   = static_cast<Json::Int64>(params.count);
        result_items.append(item_data);
    }
    log_data["result_items"] = result_items;
    owner->server.log.write("item_combine", log_data);

    co_return true;
}

std::map<EQUIPMENT_PARTS, std::shared_ptr<equipment>> items::equipments() const
{
    return equipment_map_t{
        {EQUIPMENT_PARTS::WEAPON,     _weapon                                                  },
        {EQUIPMENT_PARTS::ARMOR,      _armor                                                   },
        {EQUIPMENT_PARTS::SHIELD,     _shield                                                  },
        {EQUIPMENT_PARTS::HELMET,     _helmet                                                  },
        {EQUIPMENT_PARTS::LEFT_HAND,  _rings[static_cast<int>(EQUIPMENT_POSITION::LEFT)]       },
        {EQUIPMENT_PARTS::RIGHT_HAND, _rings[static_cast<int>(EQUIPMENT_POSITION::RIGHT)]      },
        {EQUIPMENT_PARTS::LEFT_AUX,   _auxiliaries[static_cast<int>(EQUIPMENT_POSITION::LEFT)] },
        {EQUIPMENT_PARTS::RIGHT_AUX,  _auxiliaries[static_cast<int>(EQUIPMENT_POSITION::RIGHT)]}
    };
}

std::shared_ptr<equipment> items::equipment(EQUIPMENT_PARTS parts) const
{
    switch (parts)
    {
    case EQUIPMENT_PARTS::WEAPON:
        return this->weapon();
    case EQUIPMENT_PARTS::ARMOR:
        return this->armor();
    case EQUIPMENT_PARTS::SHIELD:
        return this->shield();
    case EQUIPMENT_PARTS::HELMET:
        return this->helmet();
    case EQUIPMENT_PARTS::LEFT_HAND:
        return this->ring(EQUIPMENT_POSITION::LEFT);
    case EQUIPMENT_PARTS::RIGHT_HAND:
        return this->ring(EQUIPMENT_POSITION::RIGHT);
    case EQUIPMENT_PARTS::LEFT_AUX:
        return this->auxiliary(EQUIPMENT_POSITION::LEFT);
    case EQUIPMENT_PARTS::RIGHT_AUX:
        return this->auxiliary(EQUIPMENT_POSITION::RIGHT);
    default:
        return nullptr;
    }
}

bool items::reserved(uint8_t index) const
{
    return this->_escrow.reserved(index);
}

const items::escrow_type& items::escrow() const
{
    return this->_escrow;
}

uint16_t items::locked_count(const fb::game::item& item) const
{
    for (auto i : this->_escrow.indices())
    {
        if (this->at(i).get() == &item)
            return this->_escrow.at(i)->count();
    }
    return 0;
}

void items::load(const fb::protocol::internal::Escrow& row)
{
    this->_escrow.restore(row.id, row.money);
}

bool items::hold(uint64_t escrow_id, uint64_t money, const std::vector<escrow_part>& parts)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    owner->assert_thread();

    if (this->_escrow.contains(escrow_id) || owner->money() < money)
        return false;

    auto claimed = std::array<bool, CONTAINER_CAPACITY>{};
    for (const auto& part : parts)
    {
        if (part.index > CONTAINER_CAPACITY - 1 || claimed[part.index] || this->_escrow.reserved(part.index))
            return false;

        auto item = this->at(part.index);
        if (item == nullptr || part.count == 0 || item->count() - item->trade_count() < part.count)
            return false;

        claimed[part.index] = true;
    }

    // Every part was checked above, so splitting cannot leave the hold half done.
    auto held = std::vector<escrow_type::part>{};
    held.reserve(parts.size());
    for (const auto& part : parts)
    {
        auto item  = this->at(part.index);
        auto split = item->split(part.count);
        if (split == item)
            std::ignore = inventory<fb::game::item>::remove(part.index);
        split->container(this);
        held.push_back(escrow_type::part{part.index, split});
    }
    std::ignore = this->_escrow.hold(escrow_id, money, held);

    // The display shows usable and held money together, so the hold is counted before the usable money drops.
    owner->money_reduce(money);
    return true;
}

void items::release(uint64_t escrow_id)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    owner->assert_thread();

    auto held = this->_escrow.remove(escrow_id);
    if (held.has_value() == false)
        return;

    for (const auto& part : held->parts)
    {
        auto regular = this->at(part.index);
        if (regular == nullptr)
        {
            std::ignore = inventory<fb::game::item>::add(part.value, part.index);
        }
        else
        {
            regular->count(regular->count() + part.value->count());
            part.value->container(nullptr);
        }
    }

    if (held->money > 0)
        std::ignore = owner->money_add(held->money);
}

void items::take(uint64_t escrow_id)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    owner->assert_thread();

    auto held = this->_escrow.remove(escrow_id);
    if (held.has_value() == false)
        return;

    for (const auto& part : held->parts)
    {
        part.value->container(nullptr);
        if (this->at(part.index) == nullptr)
            owner->listener.on_item_remove(*owner, part.index, ITEM_DELETE_TYPE::REMOVED);
        else
            owner->listener.on_item_update(*owner, part.index);
    }

    if (held->money > 0)
        owner->update(UPDATE_STATE_LEVEL::EXP_MONEY);
}

// Test support: leaves every escrow entry with only its money, as an escrow item row that fails to restore does.
void items::drop_escrow_items()
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    owner->assert_thread();

    auto entries = this->_escrow.entries();
    for (const auto& [escrow_id, money] : entries)
    {
        auto held = this->_escrow.remove(escrow_id);
        if (held.has_value() == false)
            continue;

        for (const auto& part : held->parts)
        {
            part.value->container(nullptr);
            if (this->at(part.index) == nullptr)
                owner->listener.on_item_remove(*owner, part.index, ITEM_DELETE_TYPE::REMOVED);
        }
        std::ignore = this->_escrow.hold(escrow_id, held->money, {});
    }
}