#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/game/item.h>
#include <fb/game/map.h>
#include <fb/encoding.h>
#include <json/json.h>

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
        equipment = this->_weapon;
        if (this->_weapon != nullptr)
            this->_weapon = nullptr;
        break;

    case EQUIPMENT_PARTS::ARMOR:
        equipment = this->_armor;
        if (this->_armor != nullptr)
            this->_armor = nullptr;
        break;

    case EQUIPMENT_PARTS::SHIELD:
        equipment = this->_shield;
        if (this->_shield != nullptr)
            this->_shield = nullptr;
        break;

    case EQUIPMENT_PARTS::HELMET:
        equipment = this->_helmet;
        if (this->_helmet != nullptr)
            this->_helmet = nullptr;
        break;

    case EQUIPMENT_PARTS::LEFT_HAND:
        equipment = this->_rings[0];
        if (this->_rings[0] != nullptr)
            this->_rings[0] = nullptr;
        break;

    case EQUIPMENT_PARTS::RIGHT_HAND:
        equipment = this->_rings[1];
        if (this->_rings[1] != nullptr)
            this->_rings[1] = nullptr;
        break;

    case EQUIPMENT_PARTS::LEFT_AUX:
        equipment = this->_auxiliaries[0];
        if (this->_auxiliaries[0] != nullptr)
            this->_auxiliaries[0] = nullptr;
        break;

    case EQUIPMENT_PARTS::RIGHT_AUX:
        equipment = this->_auxiliaries[1];
        if (this->_auxiliaries[1] != nullptr)
            this->_auxiliaries[1] = nullptr;
        break;
    }

    if (equipment == nullptr)
        return nullptr;

    owner->update(UPDATE_STATE_LEVEL::ALL);

    // Execute equipment deactivation script
    auto& model = equipment->based<fb::model::equipment>();
    if (model.on_deactivated.empty() == false)
    {
        auto lua = fb::lua::new_context();
        if (lua != nullptr)
        {
#if defined DEBUG || defined _DEBUG
            lua->load(model.script);
#endif
            lua->func(model.on_deactivated);
            lua->pushobject(*owner);
            lua->pushinteger(parts);
            lua->pushobject(*equipment);
            std::ignore = lua->call(3);
        }
    }

    // Call listener for packet response
    owner->listener.on_equipment_off(*owner, parts, *equipment);

    owner->update_external(true);
    return equipment;
}

uint8_t items::add(std::shared_ptr<item> item)
{
    auto&& result = this->add(std::vector<std::shared_ptr<fb::game::item>>{item});
    if (result.empty())
        return 0xFF;
    else
        return result[0];
}

std::vector<uint8_t> items::add(const std::vector<std::shared_ptr<item>>& items, bool stop_if_remained)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return std::vector<uint8_t>();

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
                auto diff = fb::model::datetime() - drop_time.value();
                if (diff < fb::model::const_value::death_penalty::warmth_time)
                {
                    owner->message(_TEXT(MESSAGE_ITEM_DEATH_PENALTY_WARMTH));
                    break;
                }
            }
        }

        auto& model = item->based<fb::model::item>();
        if (model.attr(ITEM_ATTRIBUTE::CASH))
        {
            auto cash   = std::static_pointer_cast<fb::game::cash>(item);
            auto before = cash->value;
            auto remain = owner->money_add(cash->value);
            if (remain != before)
            {
                if (remain > 0)
                {
                    auto new_cash = cash->replace(remain);
                    if (new_cash != nullptr)
                        std::ignore = new_cash->map(owner->map(), owner->position());
                }
                else
                {
                    std::ignore = cash->destroy();
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
                exists->merge(item);

                auto index = this->index(exists);
                updates.insert({index, exists});
                indices.push_back(index);

                if (item->empty())
                    std::ignore = item->destroy();
                else if (stop_if_remained)
                    break;
            }
            else
            {
                auto index = this->next();
                if (index == 0xFF)
                {
                    owner->message(_TEXT(MESSAGE_ITEM_FULL));
                    break;
                }

                std::ignore = this->add(item, index);

                if (item->_map != nullptr)
                    std::ignore = item->map(nullptr);

                updates.insert({index, item});
                indices.push_back(index);
            }
        }
    }
    return std::ref(indices);
}

uint8_t items::add(std::shared_ptr<item> item, uint8_t index)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0xFF;

    if (super::add(item, index) == 0xFF)
        return 0xFF;

    item->container(this);
    item->death_uid(std::nullopt);
    if (item->empty() == false)
    {
        owner->listener.on_item_update(*owner, index);

        // Log item gain event (only for non-cash items)
        auto& model = item->based<fb::model::item>();
        if (!model.attr(ITEM_ATTRIBUTE::CASH))
        {
            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
            log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
            log_data["item_id"]        = static_cast<Json::Int64>(model.id);
            log_data["item_name"]      = UTF8(model.name, PLATFORM::WINDOWS);
            log_data["count"]          = static_cast<Json::Int64>(item->count());
            log_data["index"]          = index;
            owner->server.log.write("item_gain", log_data);
        }
    }

    return index;
}

bool items::store(std::shared_ptr<item> item)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    owner->assert_thread();

    auto& model     = item->based<fb::model::item>();
    auto  item_id   = model.id;
    auto  item_name = model.name;
    auto  count     = item->count();

    if (model.attr(ITEM_ATTRIBUTE::BUNDLE))
    {
        auto found = std::find_if(this->_stored.begin(),
                                  this->_stored.end(),
                                  [&item](const std::shared_ptr<fb::game::item>& stored) {
                                      auto& model = stored->template based<fb::model::item>();
                                      return item->based<fb::model::item>() == model;
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
    owner->server.log.write("item_deposit", log_data);

    return true;
}

bool items::store(uint8_t index, uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    owner->assert_thread();

    auto item = this->at(index);
    if (item == nullptr)
        return false;

    if (item->count() < count)
        return false;

    auto deleted = this->remove(item, count, ITEM_DELETE_TYPE::NONE, false);
    auto result  = this->store(deleted);
    if (result == false)
        this->add(deleted);

    return result;
}

bool items::store(std::string_view name, uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    owner->assert_thread();

    auto item = this->find(name);
    if (item == nullptr)
        return false;

    auto index = this->index(item);
    if (index == 0xFF)
        return false;

    return this->store(index, count);
}

std::shared_ptr<item> items::stored(const fb::model::item& item) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    owner->assert_thread();

    for (const auto& stored : this->_stored)
    {
        if (stored->template based<fb::model::item>() == item)
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

std::shared_ptr<item> items::retrieve(uint8_t index, uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    owner->assert_thread();

    if (index > this->_stored.size() - 1)
        return nullptr;

    if (this->free() == false)
        return nullptr;

    auto stored       = this->_stored.at(index);
    auto stored_count = stored->count();
    if (stored_count < count)
        return nullptr;

    auto& model     = stored->based<fb::model::item>();
    auto  item_id   = model.id;
    auto  item_name = model.name;

    auto exists = model.attr(ITEM_ATTRIBUTE::BUNDLE) ? this->find(model) : nullptr;
    if (exists != nullptr)
    {
        if (exists->count() + count > model.capacity)
            return nullptr;

        stored->count(stored_count - count);
        auto added_slot = this->add(stored->based<fb::model::item>().make(owner->server, count));
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

        return retrieved;
    }
    else
    {
        if (this->free() == false)
            return nullptr;

        auto item = stored->split(count);
        if (item == stored)
            this->_stored.erase(this->_stored.begin() + index);

        this->add(item);

        // Log item retrieve event
        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
        log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
        log_data["item_id"]        = static_cast<Json::Int64>(item_id);
        log_data["item_name"]      = UTF8(item_name, PLATFORM::WINDOWS);
        log_data["count"]          = static_cast<Json::Int64>(count);
        owner->server.log.write("item_retrieve", log_data);

        return item;
    }
}

std::shared_ptr<item> items::retrieve(std::string_view name, uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    owner->assert_thread();

    for (size_t i = 0; i < this->_stored.size(); ++i)
    {
        auto& model = this->_stored[i]->template based<fb::model::item>();
        if (model.name == name)
            return this->retrieve(static_cast<uint8_t>(i), count);
    }
    return nullptr;
}

std::shared_ptr<item> items::retrieve(const fb::model::item& item, uint16_t count)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    owner->assert_thread();

    for (size_t i = 0; i < this->_stored.size(); ++i)
    {
        auto& model = this->_stored[i]->template based<fb::model::item>();
        if (model == item)
            return this->retrieve(static_cast<uint8_t>(i), count);
    }
    return nullptr;
}

uint32_t items::deposited() const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0;

    owner->assert_thread();

    return this->_deposited;
}

void items::deposited(uint32_t value)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    owner->assert_thread();

    this->_deposited = value;
}

uint32_t items::deposit(uint32_t value)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0;

    owner->assert_thread();

    uint32_t capacity         = 0xFFFFFFFF - this->_deposited;
    uint32_t lack             = 0;
    uint32_t deposited_amount = 0;
    if (value > capacity)
    {
        deposited_amount = capacity;
        this->deposited(this->_deposited + capacity);
        lack = value - capacity;
    }
    else
    {
        deposited_amount = value;
        this->deposited(this->_deposited + value);
    }

    // Log money deposit event
    if (deposited_amount > 0)
    {
        auto log_data                 = Json::Value();
        log_data["character_id"]      = static_cast<Json::Int64>(owner->id);
        log_data["character_name"]    = UTF8(owner->name(), PLATFORM::WINDOWS);
        log_data["amount"]            = static_cast<Json::Int64>(deposited_amount);
        log_data["current_deposited"] = static_cast<Json::Int64>(this->_deposited);
        owner->server.log.write("money_deposit", log_data);
    }

    return lack;
}

uint32_t items::withdraw(uint32_t value)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0;

    owner->assert_thread();

    uint32_t lack             = 0;
    uint32_t withdrawn_amount = 0;
    if (this->_deposited < value)
    {
        withdrawn_amount = this->_deposited;
        lack             = value - this->_deposited;
        this->deposited(0);
    }
    else
    {
        withdrawn_amount = value;
        this->deposited(this->_deposited - value);
    }

    // Log money withdraw event
    if (withdrawn_amount > 0)
    {
        auto log_data                 = Json::Value();
        log_data["character_id"]      = static_cast<Json::Int64>(owner->id);
        log_data["character_name"]    = UTF8(owner->name(), PLATFORM::WINDOWS);
        log_data["amount"]            = static_cast<Json::Int64>(withdrawn_amount);
        log_data["current_deposited"] = static_cast<Json::Int64>(this->_deposited);
        owner->server.log.write("money_withdraw", log_data);
    }

    return lack;
}

std::shared_ptr<item> items::active(uint8_t index)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    try
    {
        owner->assert_state({STATE::RIDING, STATE::GHOST});

        auto item = this->at(index);
        if (item == nullptr)
            return nullptr;

        auto weak = item->weak_from_this_as<fb::game::item>();
        item->active();
        if (weak.lock() == nullptr)
            return nullptr;

        if (item->empty())
        {
            std::ignore = item->destroy();
            return nullptr;
        }
        else
        {
            return item;
        }
    }
    catch (std::exception& e)
    {
        owner->message(e.what());
    }
    return nullptr;
}

uint8_t items::inactive(EQUIPMENT_PARTS parts)
{
    if (this->free() == false)
        return 0xFF;

    auto item = this->equipment_off(parts);
    if (item == nullptr)
        return 0xFF;

    auto slot = this->add(item);
    return slot;
}

uint8_t items::index(const fb::model::item& model) const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto now = this->at(i);

        if (now == nullptr)
            continue;

        if (now->based<fb::model::item>() == model)
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

        if (now->based<fb::model::item>() == item->based<fb::model::item>())
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

std::shared_ptr<equipment> items::wear(EQUIPMENT_PARTS parts, std::shared_ptr<equipment> item)
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
    owner->update_external(false);
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
    owner->update_external(false);

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
    owner->update_external(false);

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
    owner->update_external(false);

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
    owner->update_external(false);

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
    owner->update_external(false);

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

        auto& model = item->based<fb::model::item>();
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

        if (item->based<fb::model::item>() == model)
            return std::static_pointer_cast<fb::game::item>(item);
    }

    for (auto& [parts, equipment] : this->equipments())
    {
        if (equipment == nullptr)
            continue;

        if (equipment->based<fb::model::item>() == model)
            return std::static_pointer_cast<fb::game::item>(equipment);
    }

    return nullptr;
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
            if (slot_item->based<fb::model::item>() == model)
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
            if (slot_item != nullptr && slot_item->based<fb::model::item>() == model)
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
        const auto& model = slot_item->based<fb::model::item>();
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

std::shared_ptr<item> items::drop(uint8_t index, uint8_t count, bool action, ITEM_DELETE_TYPE delete_type)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    try
    {
        if (action)
            owner->assert_state({STATE::RIDING, STATE::GHOST});

        auto item = this->at(index);
        if (item == nullptr)
            return nullptr;

        auto& model = item->based<fb::model::item>();
        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_CANNOT_DROP_ITEM));

        auto dropped = this->remove(item, count, delete_type);
        if (dropped != nullptr)
        {
            std::ignore = dropped->map(owner->map(), owner->position());

            if (action)
                owner->action(ACTION::PICKUP, DURATION::PICKUP);
        }

        return dropped;
    }
    catch (std::exception& e)
    {
        owner->message(e.what());
    }
    return nullptr;
}

void items::loot(bool boost)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    try
    {
        auto map = owner->map();
        if (map == nullptr)
            return;

        auto lua = fb::lua::new_context();
        if (lua != nullptr)
        {
#if defined DEBUG || defined _DEBUG
            lua->load("scripts/interaction.lua");
#endif
            lua->func("on_loot");
            lua->pushobject(*owner);
            std::ignore = lua->call(1);
        }

        owner->assert_state({STATE::GHOST, STATE::RIDING});
        owner->action(ACTION::PICKUP, DURATION::PICKUP);

        // Pick up items in reverse order
        auto belows = std::vector<std::shared_ptr<item>>();
        for (auto& below : map->belows(owner->position(), OBJECT_TYPE::ITEM))
            belows.push_back(std::static_pointer_cast<item>(below));

        if (belows.size() == 0)
        {
        }
        else if (boost)
        {
            std::ignore = owner->items.add(belows, true);
        }
        else
        {
            std::ignore = owner->items.add(belows.front());
        }
    }
    catch (std::exception& e)
    {
        owner->message(e.what());
    }
}

bool items::throws(uint8_t index, bool all)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    try
    {
        auto item = owner->items.at(index);
        if (item == nullptr)
            return false;

        auto& model = item->based<fb::model::item>();
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
        std::ignore = dropped->map(map, position);
        return true;
    }
    catch (std::exception& e)
    {
        owner->message(e.what());
    }
    return false;
}

std::shared_ptr<item> items::remove(uint8_t index, uint16_t count, ITEM_DELETE_TYPE attr, bool detach)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    auto item = this->at(index);
    if (item == nullptr)
        return nullptr;

    auto splitted = item->split(count);
    if (splitted == item)
    {
        if (detach)
            splitted->_container = nullptr;

        std::ignore = inventory<fb::game::item>::remove(index);
        owner->listener.on_item_remove(*owner, index, attr);

        // Log item remove event (only for non-cash items)
        auto& model = item->based<fb::model::item>();
        if (!model.attr(ITEM_ATTRIBUTE::CASH))
        {
            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
            log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
            log_data["item_id"]        = static_cast<Json::Int64>(model.id);
            log_data["item_name"]      = UTF8(model.name, PLATFORM::WINDOWS);
            log_data["count"]          = static_cast<Json::Int64>(count);
            log_data["index"]          = index;
            log_data["delete_type"]    = static_cast<int>(attr);
            owner->server.log.write("item_remove", log_data);
        }
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

bool items::swap(uint8_t src, uint8_t dst)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    if (inventory<item>::swap(src, dst) == false)
        return false;

    const auto right = this->at(src);
    if (right != nullptr)
        owner->listener.on_item_update(*owner, src);
    else
        owner->listener.on_item_remove(*owner, src);

    const auto left = this->at(dst);
    if (left != nullptr)
        owner->listener.on_item_update(*owner, dst);
    else
        owner->listener.on_item_remove(*owner, dst);

    owner->listener.on_item_swap(*owner, src, dst);
    return true;
}

bool items::is_rewardable(const std::unordered_map<uint32_t, uint16_t>& items, uint32_t money) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    auto money_cap = 0xFFFFFFFF - owner->money();
    if (money_cap < money)
        return false;

    auto required_size = 0;
    for (auto& [id, count] : items)
    {
        auto& model = table::item[id];
        if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
            required_size += count;
        else
            required_size++;
    }

    auto free_size = this->free_size();
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->at(i);
        if (item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
            continue;

        if (items.contains(model.id) == false)
            continue;

        if (model.capacity < item->count() + items.at(model.id))
            return false;

        free_size++;
    }

    return free_size >= required_size;
}

bool items::is_rewardable(const std::vector<fb::model::dsl>& items) const
{
    auto buffer = std::unordered_map<uint32_t, uint16_t>{};
    auto money  = 0;
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

exchange_result items::exchange(const std::unordered_map<uint32_t, uint16_t>& cost_items,
                                uint32_t                                      cost_money,
                                const std::unordered_map<uint32_t, uint16_t>& reward_items,
                                uint32_t                                      reward_money)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return exchange_result::lack_cost;

    if (owner->money() < cost_money)
        return exchange_result::lack_cost;

    auto effective_free_slots       = this->free_size();
    auto simulated_bundle_remaining = std::unordered_map<uint32_t, uint16_t>{};
    auto slots_by_id                = std::unordered_map<uint32_t, std::vector<uint8_t>>{};

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto slot = this->at(static_cast<uint8_t>(i));
        if (slot == nullptr)
            continue;
        auto& model = slot->based<fb::model::item>();
        slots_by_id[model.id].push_back(static_cast<uint8_t>(i));
    }

    for (auto& [id, cost_count] : cost_items)
    {
        auto& model = table::item[id];
        auto  it    = slots_by_id.find(id);
        if (it == slots_by_id.end() || it->second.empty())
            return exchange_result::lack_cost;

        if (model.attr(ITEM_ATTRIBUTE::BUNDLE))
        {
            auto index = it->second[0];
            auto slot  = this->at(index);
            if (slot == nullptr)
                return exchange_result::lack_cost;

            auto current = slot->count();
            if (current < cost_count)
                return exchange_result::lack_cost;

            auto remain = static_cast<uint16_t>(current - cost_count);
            if (remain == 0)
                effective_free_slots++;
            else
                simulated_bundle_remaining[id] = remain;
        }
        else
        {
            auto slot_count = static_cast<uint32_t>(it->second.size());
            if (slot_count < cost_count)
                return exchange_result::lack_cost;

            effective_free_slots =
                static_cast<uint8_t>(std::min(0xFF, static_cast<int>(effective_free_slots) + cost_count));
        }
    }

    uint32_t money_after         = owner->money() - cost_money;
    uint32_t effective_money_cap = 0xFFFFFFFF - money_after;
    if (effective_money_cap < reward_money)
        return exchange_result::lack_capacity;

    int required_size = 0;
    for (auto& [id, count] : reward_items)
    {
        auto& model = table::item[id];
        if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
            required_size += count;
        else
            required_size++;
    }

    int available_slots = effective_free_slots;
    for (auto& [id, count] : reward_items)
    {
        auto& model = table::item[id];
        if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
            continue;

        auto     it       = simulated_bundle_remaining.find(id);
        uint16_t existing = (it != simulated_bundle_remaining.end()) ? it->second : 0;
        if (model.capacity < existing + count)
            return exchange_result::lack_capacity;

        available_slots++;
    }

    if (available_slots < required_size)
        return exchange_result::lack_capacity;

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
            if (slot == nullptr || slot->based<fb::model::item>().id != id)
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
            auto     removed   = this->remove(index, to_remove, ITEM_DELETE_TYPE::GIVE, true);
            if (removed != nullptr)
                std::ignore = removed->destroy();
            remaining -= to_remove;

            if (slot->count() == 0)
                ++slot_it;
        }
    }

    owner->money_add(reward_money);

    for (auto& [id, count] : reward_items)
    {
        auto& model = table::item[id];
        auto  made  = model.make(owner->server, count);
        if (made != nullptr)
        {
            auto idx = this->add(made);
            if (idx == 0xFF)
                std::ignore = made->destroy();
        }
    }

    return exchange_result::ok;
}

std::map<EQUIPMENT_PARTS, std::shared_ptr<equipment>> items::equipments() const
{
    return std::map<EQUIPMENT_PARTS, std::shared_ptr<equipment>>{
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