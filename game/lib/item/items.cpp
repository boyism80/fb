#include <fb/game/character.h>
#include <fb/game/context.h>
#include <fb/game/item.h>
#include <fb/game/map.h>

fb::game::items::items(fb::game::character& owner) :
    inventory(owner),
    owner(owner)
{ }

fb::game::items::~items()
{
    for (auto item : *this)
    {
        if (item != nullptr)
            delete item;
    }

    for (auto& [parts, equipment] : this->equipments())
    {
        if (equipment != nullptr)
            delete equipment;
    }
}

fb::game::equipment* fb::game::items::equipment_off(EQUIPMENT_PARTS parts)
{
    auto equipment = (fb::game::equipment*)nullptr;
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

    this->owner.update(STATE_LEVEL::LEVEL_MAX);
    owner.listener.on_equipment_off(this->owner, parts, *equipment);

    this->owner.update_external(false);
    return equipment;
}

uint8_t fb::game::items::add(fb::game::item& item)
{
    return this->add(&item);
}

uint8_t fb::game::items::add(fb::game::item* item)
{
    auto&& result = this->add(std::vector<fb::game::item*>{item});
    if (result.empty())
        return 0xFF;
    else
        return result[0];
}

std::vector<uint8_t> fb::game::items::add(const std::vector<fb::game::item*>& items, bool stop_if_remained)
{
    auto indices = std::vector<uint8_t>();
    auto updates = fb::game::item::container();

    for (auto item : items)
    {
        if (item == nullptr)
            continue;

        auto death_cid = item->death_cid();
        if (death_cid.has_value() && death_cid.value() != this->owner.id())
        {
            auto& drop_time = item->dropped_time();
            if (drop_time.has_value())
            {
                auto diff = fb::model::datetime() - drop_time.value();
                if (diff < fb::model::const_value::death_penalty::warmth_time)
                {
                    this->owner.message("죽은 자의 온기가 남아있습니다.");
                    break;
                }
            }
        }

        auto& model = item->based<fb::model::item>();
        if (model.attr(ITEM_ATTRIBUTE::CASH))
        {
            auto cash   = static_cast<fb::game::cash*>(item);
            auto before = cash->value;
            auto remain = this->owner.money_add(cash->value);
            if (remain != before)
            {
                if (remain > 0)
                    cash->replace(remain)->map(this->owner.map(), this->owner.position()); // 먹고 남은 돈으로 설정
                else
                    std::ignore = cash->destroy();
            }

            this->owner.update(STATE_LEVEL::EXP_MONEY);
            if (remain != 0)
            {
                this->owner.message(_TEXT(MESSAGE_MONEY_FULL));
                if (stop_if_remained)
                    break;
            }
        }
        else
        {
            auto exists = model.attr(ITEM_ATTRIBUTE::BUNDLE) ? this->find(model) : nullptr;
            if (exists != nullptr)
            {
                exists->merge(*item);

                auto index = this->index(*exists);
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
                    break;

                std::ignore = this->add(*item, index);

                if (item->_map != nullptr)
                    std::ignore = item->map(nullptr);

                updates.insert({index, item});
                indices.push_back(index);
            }
        }
    }
    return std::ref(indices);
}

uint8_t fb::game::items::add(fb::game::item& item, uint8_t index)
{
    if (fb::game::inventory<fb::game::item>::add(item, index) == 0xFF)
        return 0xFF;

    item._container = this;
    item._death_cid = std::nullopt;
    if (item.empty() == false)
        this->owner.listener.on_item_update(static_cast<character&>(this->owner), index);

    return index;
}

bool fb::game::items::store(fb::game::item& item)
{
    this->owner.assert_thread();

    if (item.based<fb::model::item>().attr(ITEM_ATTRIBUTE::BUNDLE))
    {
        auto found = std::find_if(this->_stored.begin(), this->_stored.end(), [&item](auto* stored) {
            auto& model = stored->template based<fb::model::item>();
            return item.based<fb::model::item>() == model;
        });

        if (found == this->_stored.end())
        {
            this->_stored.push_back(&item);
        }
        else
        {
            auto stored   = *found;
            auto capacity = 0xFFFF - stored->count();
            if (item.count() > capacity)
                return false;

            stored->count(stored->count() + item.count());
        }
    }
    else
    {
        this->_stored.push_back(&item);
    }

    item._container = this;
    return true;
}

bool fb::game::items::store(uint8_t index, uint16_t count)
{
    this->owner.assert_thread();

    auto item = this->at(index);
    if (item == nullptr)
        return false;

    if (item->count() < count)
        return false;

    auto deleted = this->remove(*item, count, ITEM_DELETE_TYPE::NONE, false);
    auto result  = this->store(*deleted);
    if (result == false)
        this->add(deleted);

    return result;
}

bool fb::game::items::store(const std::string& name, uint16_t count)
{
    this->owner.assert_thread();

    auto item = this->find(name);
    if (item == nullptr)
        return false;

    auto index = this->index(*item);
    if (index == 0xFF)
        return false;

    return this->store(index, count);
}

item* fb::game::items::stored(const fb::model::item& item) const
{
    this->owner.assert_thread();

    auto found = std::find_if(this->_stored.cbegin(), this->_stored.cend(), [&item](auto* stored) {
        return stored->template based<fb::model::item>() == item;
    });

    if (found == this->_stored.cend())
        return nullptr;

    return *found;
}

const std::vector<item*>& fb::game::items::stored() const
{
    this->owner.assert_thread();

    return this->_stored;
}

item* fb::game::items::retrieve(uint8_t index, uint16_t count)
{
    this->owner.assert_thread();

    if (index > this->_stored.size() - 1)
        return nullptr;

    if (this->free() == false)
        return nullptr;

    auto stored       = this->_stored.at(index);
    auto stored_count = stored->count();
    if (stored_count < count)
        return nullptr;

    auto& model  = stored->based<fb::model::item>();
    auto  exists = model.attr(ITEM_ATTRIBUTE::BUNDLE) ? this->find(model) : nullptr;
    if (exists != nullptr)
    {
        if (exists->count() + count > model.capacity)
            return nullptr;

        stored->count(stored_count - count);
        auto added_slot = this->add(stored->based<fb::model::item>().make(this->owner.context, count));
        if (stored->empty())
        {
            auto i = this->_stored.begin() + index;
            this->_stored.erase(i);
        }

        return this->at(added_slot);
    }
    else
    {
        if (this->free() == false)
            return nullptr;

        auto item = stored->split(count);
        if (stored->empty())
            this->_stored.erase(this->_stored.begin() + index);

        this->add(item);
        return item;
    }
}

item* fb::game::items::retrieve(const std::string& name, uint16_t count)
{

    this->owner.assert_thread();

    auto found = std::find_if(this->_stored.begin(), this->_stored.end(), [&name](auto* stored) {
        auto& model = stored->template based<fb::model::item>();
        return model.name == name;
    });

    if (found == this->_stored.end())
        return nullptr;

    auto index = std::distance(this->_stored.begin(), found);
    return this->retrieve((uint8_t)index, count);
}

item* fb::game::items::retrieve(const fb::model::item& item, uint16_t count)
{
    this->owner.assert_thread();

    auto found = std::find_if(this->_stored.begin(), this->_stored.end(), [&item](auto* stored) {
        auto& model = stored->template based<fb::model::item>();
        return model == item;
    });

    if (found == this->_stored.end())
        return nullptr;

    auto index = std::distance(this->_stored.begin(), found);
    return this->retrieve((uint8_t)index, count);
}

uint32_t fb::game::items::deposited() const
{
    this->owner.assert_thread();

    return this->_deposited;
}

void fb::game::items::deposited(uint32_t value)
{
    this->owner.assert_thread();

    this->_deposited = value;
}

uint32_t fb::game::items::deposit(uint32_t value)
{
    this->owner.assert_thread();

    uint32_t capacity = 0xFFFFFFFF - this->_deposited;
    uint32_t lack     = 0;
    if (value > capacity)
    {
        this->deposited(this->_deposited + capacity);
        lack = value - capacity;
    }
    else
    {
        this->deposited(this->_deposited + value);
    }

    return lack;
}

uint32_t fb::game::items::withdraw(uint32_t value)
{
    this->owner.assert_thread();

    uint32_t lack = 0;
    if (this->_deposited < value)
    {
        lack = value - this->_deposited;
        this->deposited(0);
    }
    else
    {
        this->deposited(this->_deposited - value);
    }

    return lack;
}

fb::game::item* fb::game::items::active(uint8_t index)
{
    auto& context = this->owner.context;

    try
    {
        this->owner.assert_state({STATE::RIDING, STATE::GHOST});

        auto item = this->at(index);
        if (item == nullptr)
            return nullptr;

        item->active();
        if (context.alive(*item) == false)
        {
            return nullptr;
        }
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
        this->owner.message(e.what());
    }
    return nullptr;
}

uint8_t fb::game::items::inactive(EQUIPMENT_PARTS parts)
{
    if (this->free() == false)
        return 0xFF;

    auto item = this->equipment_off(parts);
    if (item == nullptr)
        return 0xFF;

    auto slot = this->add(item);
    return slot;
}

uint8_t fb::game::items::index(const fb::model::item& item) const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto now = this->at(i);

        if (now == nullptr)
            continue;

        if (now->based<fb::model::item>() == item)
            return i;
    }

    return 0xFF;
}

uint8_t fb::game::items::index(const fb::game::item& item) const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto now = this->at(i);
        if (now == &item)
            return i;
    }

    return 0xFF;
}

std::vector<uint8_t> fb::game::items::index_all(const fb::model::item& item) const
{
    auto result = std::vector<uint8_t>();
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto now = this->at(i);

        if (now == nullptr)
            continue;

        if (now->based<fb::model::item>() == item)
            result.push_back(i);
    }

    return result;
}

bool fb::game::items::update(uint8_t index) const
{
    auto item = this->at(index);
    if (item == nullptr)
        return false;

    auto remained = item->count() - item->trade_count();
    if (remained == 0)
        this->owner.listener.on_item_remove(this->owner, index, ITEM_DELETE_TYPE::NONE);
    else
        this->owner.listener.on_item_update(this->owner, index);
    return true;
}

fb::game::equipment* fb::game::items::wear(EQUIPMENT_PARTS parts, fb::game::equipment* item)
{
    if (item != nullptr)
        item->_container = this;

    switch (parts) // EQUIPMENT_PARTS
    {
    case EQUIPMENT_PARTS::WEAPON:
        return this->owner.items.weapon(static_cast<fb::game::weapon*>(item));

    case EQUIPMENT_PARTS::ARMOR:
        return this->owner.items.armor(static_cast<fb::game::armor*>(item));

    case EQUIPMENT_PARTS::SHIELD:
        return this->owner.items.shield(static_cast<fb::game::shield*>(item));

    case EQUIPMENT_PARTS::HELMET:
        return this->owner.items.helmet(static_cast<fb::game::helmet*>(item));

    case EQUIPMENT_PARTS::LEFT_HAND:
        return this->owner.items.ring(static_cast<fb::game::ring*>(item), EQUIPMENT_POSITION::LEFT);

    case EQUIPMENT_PARTS::RIGHT_HAND:
        return this->owner.items.ring(static_cast<fb::game::ring*>(item), EQUIPMENT_POSITION::RIGHT);

    case EQUIPMENT_PARTS::LEFT_AUX:
        return this->owner.items.auxiliary(static_cast<fb::game::auxiliary*>(item), EQUIPMENT_POSITION::LEFT);

    case EQUIPMENT_PARTS::RIGHT_AUX:
        return this->owner.items.auxiliary(static_cast<fb::game::auxiliary*>(item), EQUIPMENT_POSITION::RIGHT);

    default:
        throw std::runtime_error("invalid equipment parts");
    }
}

fb::game::weapon* fb::game::items::weapon() const
{
    return this->_weapon;
}

fb::game::weapon* fb::game::items::weapon(fb::game::weapon* weapon)
{
    auto before = this->_weapon;

    this->_weapon = weapon;
    this->owner.update_external(true);
    return before;
}

fb::game::armor* fb::game::items::armor() const
{
    return this->_armor;
}

fb::game::armor* fb::game::items::armor(fb::game::armor* armor)
{
    auto before = this->_armor;

    this->_armor = armor;
    this->owner.update_external(true);

    return before;
}

fb::game::shield* fb::game::items::shield() const
{
    return this->_shield;
}

fb::game::shield* fb::game::items::shield(fb::game::shield* shield)
{
    fb::game::shield* before = this->_shield;

    this->_shield = shield;
    this->owner.update_external(true);

    return before;
}

fb::game::helmet* fb::game::items::helmet() const
{
    return this->_helmet;
}

fb::game::helmet* fb::game::items::helmet(fb::game::helmet* helmet)
{
    fb::game::helmet* before = this->_helmet;

    this->_helmet = helmet;
    this->owner.update_external(true);

    return before;
}

fb::game::ring* fb::game::items::ring(EQUIPMENT_POSITION position) const
{
    return this->_rings[static_cast<int>(position)];
}

fb::game::ring* fb::game::items::ring(fb::game::ring* ring)
{
    fb::game::ring* before = nullptr;

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

fb::game::ring* fb::game::items::ring(fb::game::ring* ring, EQUIPMENT_POSITION position)
{
    auto before                              = this->_rings[static_cast<int>(position)];
    this->_rings[static_cast<int>(position)] = ring;
    this->owner.update_external(true);

    return before;
}

fb::game::auxiliary* fb::game::items::auxiliary(EQUIPMENT_POSITION position) const
{
    return this->_auxiliaries[static_cast<int>(position)];
}

fb::game::auxiliary* fb::game::items::auxiliary(fb::game::auxiliary* auxiliary)
{
    fb::game::auxiliary* before = nullptr;

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

fb::game::auxiliary* fb::game::items::auxiliary(fb::game::auxiliary* auxiliary, EQUIPMENT_POSITION position)
{
    auto before                                    = this->_auxiliaries[static_cast<int>(position)];
    this->_auxiliaries[static_cast<int>(position)] = auxiliary;
    this->owner.update_external(true);

    return before;
}

fb::game::item* fb::game::items::find(const std::string& name) const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->at(i);
        if (item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        if (model.name == name)
            return item;
    }

    return nullptr;
}

fb::game::item* fb::game::items::find(const fb::model::item& model) const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->at(i);
        if (item == nullptr)
            continue;

        if (item->based<fb::model::item>() == model)
            return item;
    }

    for (auto& [parts, equipment] : this->equipments())
    {
        if (equipment == nullptr)
            continue;

        if (equipment->based<fb::model::item>() == model)
            return equipment;
    }

    return nullptr;
}

fb::game::item* fb::game::items::find_bundle(const fb::model::item& model) const
{
    if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
        return nullptr;

    return this->find(model);
}

fb::game::item* fb::game::items::drop(uint8_t index, uint8_t count, bool action, ITEM_DELETE_TYPE delete_type)
{

    try
    {
        if (action)
            this->owner.assert_state({STATE::RIDING, STATE::GHOST});

        auto item = this->at(index);
        if (item == nullptr)
            return nullptr;

        auto& model = item->based<fb::model::item>();
        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_CANNOT_DROP_ITEM));

        auto dropped = this->remove(*item, count, delete_type);
        if (dropped != nullptr)
        {
            std::ignore = dropped->map(this->owner.map(), this->owner.position());

            if (action)
                this->owner.action(ACTION::PICKUP, DURATION::PICKUP);
        }

        return dropped;
    }
    catch (std::exception& e)
    {
        this->owner.message(e.what());
    }
    return nullptr;
}

void fb::game::items::pickup(bool boost)
{

    try
    {
        auto map = this->owner.map();
        if (map == nullptr)
            return;

        this->owner.assert_state({STATE::GHOST, STATE::RIDING});
        this->owner.action(ACTION::PICKUP, DURATION::PICKUP);

        // Pick up items in reverse order
        auto belows = std::vector<fb::game::item*>();
        for (auto below : map->belows(this->owner.position(), OBJECT_TYPE::ITEM))
            belows.push_back(static_cast<fb::game::item*>(below));

        if (belows.size() == 0)
        {
        }
        else if (boost)
        {
            std::ignore = this->owner.items.add(belows, true);
        }
        else
        {
            std::ignore = this->owner.items.add(belows[0]);
        }

        auto lua = fb::lua::new_context();
        if (lua != nullptr)
        {
#if defined DEBUG | defined _DEBUG
            lua->load("scripts/interaction.lua");
#endif
            lua->func("on_pickup");
            lua->pushobject(this->owner);
            std::ignore = lua->call(1);
        }
    }
    catch (std::exception& e)
    {
        this->owner.message(e.what());
    }
}

bool fb::game::items::throws(uint8_t index)
{

    try
    {
        auto item = this->owner.items.at(index);
        if (item == nullptr)
            return false;

        auto& model = item->based<fb::model::item>();
        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_CANNOT_THROW_ITEM));

        auto map = this->owner.map();
        if (map == nullptr)
            throw std::exception();

        auto dropped  = this->remove(index, 1, ITEM_DELETE_TYPE::THROW);
        auto position = this->owner.position();
        for (int i = 0; i < 7; i++)
        {
            auto before = position;
            position.forward(this->owner.direction());
            if (map->movable(position) == false)
            {
                position = before;
                break;
            }
        }

        this->owner.listener.on_item_throws(this->owner, *dropped, position);
        std::ignore = dropped->map(map, position);
        return true;
    }
    catch (std::exception& e)
    {
        this->owner.message(e.what());
    }
    return false;
}

fb::game::item* fb::game::items::remove(uint8_t index, uint16_t count, ITEM_DELETE_TYPE attr, bool detach)
{
    auto item = this->at(index);
    if (item == nullptr)
        return nullptr;

    auto splitted = item->split(count);
    if (splitted == item)
    {
        if (detach)
            splitted->_container = nullptr;

        std::ignore = fb::game::inventory<fb::game::item>::remove(index);
        this->owner.listener.on_item_remove(this->owner, index, attr);
    }

    auto current = this->at(index);
    this->owner.listener.on_item_update(this->owner, index);
    return splitted;
}

fb::game::item* fb::game::items::remove(fb::game::item& item, uint16_t count, ITEM_DELETE_TYPE attr, bool detach)
{
    auto index = this->index(item);
    if (index == 0xFF)
        return nullptr;

    return this->remove(index, count, attr, detach);
}

bool fb::game::items::swap(uint8_t src, uint8_t dst)
{
    if (fb::game::inventory<fb::game::item>::swap(src, dst) == false)
        return false;

    const auto right = this->at(src);
    if (right != nullptr)
        this->owner.listener.on_item_update(this->owner, src);
    else
        this->owner.listener.on_item_remove(this->owner, src);

    const auto left = this->at(dst);
    if (left != nullptr)
        this->owner.listener.on_item_update(this->owner, dst);
    else
        this->owner.listener.on_item_remove(this->owner, dst);

    this->owner.listener.on_item_swap(this->owner, src, dst);
    return true;
}

std::map<EQUIPMENT_PARTS, fb::game::equipment*> fb::game::items::equipments() const
{
    return std::map<EQUIPMENT_PARTS, equipment*>{
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