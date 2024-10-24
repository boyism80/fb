#include <item.h>
#include <character.h>
#include <map.h>

fb::game::items::items(fb::game::character& owner) :
    inventory(owner),
    _owner(owner)
{ }

fb::game::items::~items()
{
    for (auto item : *this)
    {
        if (item != nullptr)
            delete item;
    }

    for(auto& [parts, equipment] : this->equipments())
    {
        if(equipment != nullptr)
            delete equipment;
    }
}

async::task<uint8_t> fb::game::items::equipment_off(EQUIPMENT_PARTS parts)
{
    auto listener = this->_owner.get_listener<fb::game::character>();

    try
    {
        if (this->free() == false)
            throw std::runtime_error(message::exception::INVENTORY_OVERFLOW);

        fb::game::item* item = nullptr;
        switch (parts)
        {
        case EQUIPMENT_PARTS::WEAPON:
            item = this->_weapon;
            if (this->_weapon != nullptr)
                this->_weapon = nullptr;

            break;

        case EQUIPMENT_PARTS::ARMOR:
            item = this->_armor;
            if (this->_armor != nullptr)
                this->_armor = nullptr;
            break;

        case EQUIPMENT_PARTS::SHIELD:
            item = this->_shield;
            if (this->_shield != nullptr)
                this->_shield = nullptr;
            break;

        case EQUIPMENT_PARTS::HELMET:
            item = this->_helmet;
            if (this->_helmet != nullptr)
                this->_helmet = nullptr;
            break;

        case EQUIPMENT_PARTS::LEFT_HAND:
            item = this->_rings[0];
            if (this->_rings[0] != nullptr)
                this->_rings[0] = nullptr;
            break;

        case EQUIPMENT_PARTS::RIGHT_HAND:
            item = this->_rings[1];
            if (this->_rings[1] != nullptr)
                this->_rings[1] = nullptr;
            break;

        case EQUIPMENT_PARTS::LEFT_AUX:
            item = this->_auxiliaries[0];
            if (this->_auxiliaries[0] != nullptr)
                this->_auxiliaries[0] = nullptr;
            break;

        case EQUIPMENT_PARTS::RIGHT_AUX:
            item = this->_auxiliaries[1];
            if (this->_auxiliaries[1] != nullptr)
                this->_auxiliaries[1] = nullptr;
            break;

        default:
            throw std::runtime_error("뭐지 병신 ㅋ");
        }

        auto index = co_await this->add(item);
        if(listener != nullptr)
        {
            listener->on_updated(this->_owner, STATE_LEVEL::LEVEL_MAX);
            listener->on_show(this->_owner, false);
            listener->on_equipment_off(this->_owner, parts, index);
        }
        
        co_return index;
    }
    catch(std::exception& e)
    {
        this->_owner.message(e.what());
        co_return 0xFF;
    }
}

async::task<uint8_t> fb::game::items::add(fb::game::item& item)
{
    co_return co_await this->add(&item);
}

async::task<uint8_t> fb::game::items::add(fb::game::item* item)
{
    auto&& result = co_await this->add(std::vector<fb::game::item*> { item });
    if(result.empty())
        co_return 0xFF;
    else
        co_return result[0];
}

async::task<std::vector<uint8_t>> fb::game::items::add(const std::vector<fb::game::item*>& items, bool stop_if_remained)
{
    auto indices = std::vector<uint8_t>();
    auto updates = std::map<uint8_t, fb::game::item*>();
    auto listener = this->_owner.get_listener<fb::game::character>();

    for(auto item : items)
    {
        if(item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        if (model.attr(ITEM_ATTRIBUTE::CASH))
        {
            auto        cash = static_cast<fb::game::cash*>(item);
            auto        remain = this->_owner.money_add(cash->value);
            if (remain > 0)
                cash = co_await cash->replace(remain); // 먹고 남은 돈으로 설정
            else
                co_await cash->destroy();

            if (listener != nullptr)
                listener->on_updated(this->_owner, STATE_LEVEL::LEVEL_MIN);

            if (remain != 0)
            {
                this->_owner.message(message::money::FULL);
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
                updates.insert({ index, exists });
                indices.push_back(index);

                if (item->empty())
                    co_await item->destroy();
                else if (stop_if_remained)
                    break;
            }
            else
            {
                auto index = this->next();
                if (index == 0xFF)
                    break;

                this->add(*item, index);

                if (item->_map != nullptr)
                    co_await item->map(nullptr);

                updates.insert({ index, item });
                indices.push_back(index);
            }
        }
    }

    if(listener != nullptr)
        listener->on_item_changed(this->_owner, updates);
    co_return std::ref(indices);
}

async::task<uint8_t> fb::game::items::add(fb::game::item& item, uint8_t index)
{
    if(co_await fb::game::inventory<fb::game::item>::add(item, index) == 0xFF)
        co_return 0xFF;

    item.owner(&this->_owner);

    auto listener = this->_owner.get_listener<fb::game::character>();
    if(item.empty() == false && listener != nullptr)
        listener->on_item_update(static_cast<character&>(this->owner()), index);

    co_return index;
}

async::task< fb::game::item*> fb::game::items::active(uint8_t index)
{
    auto listener = this->_owner.get_listener<fb::game::character>();

    try
    {
        this->_owner.assert_state({STATE::RIDING, STATE::GHOST});

        auto                    item = this->at(index);
        if(item == nullptr)
            co_return nullptr;

        item->active();
        if(item->empty())
        {
            co_await item->destroy();
            co_return nullptr;
        }
        else
        {
            co_return item;
        }
    }
    catch(std::exception& e)
    {
        this->_owner.message(e.what());
        co_return nullptr;
    }
}

async::task<uint8_t> fb::game::items::inactive(EQUIPMENT_PARTS parts)
{
    co_return co_await this->equipment_off(parts);
}

uint8_t fb::game::items::index(const fb::model::item& item) const
{
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto now = this->at(i);

        if(now == nullptr)
            continue;

        if(now->based<fb::model::item>() == item)
            return i;
    }

    return 0xFF;
}

uint8_t fb::game::items::index(const fb::game::item& item) const
{
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto now = this->at(i);
        if(now == &item)
            return i;
    }

    return 0xFF;
}

std::vector<uint8_t> fb::game::items::index_all(const fb::model::item& item) const
{
    auto result = std::vector<uint8_t>();
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto now = this->at(i);

        if(now == nullptr)
            continue;

        if(now->based<fb::model::item>() == item)
            result.push_back(i);
    }

    return result;
}

fb::game::equipment* fb::game::items::wear(EQUIPMENT_PARTS parts, fb::game::equipment* item)
{
    switch(parts) // EQUIPMENT_PARTS
    {
    case EQUIPMENT_PARTS::WEAPON:
        return this->_owner.items.weapon(static_cast<fb::game::weapon*>(item));

    case EQUIPMENT_PARTS::ARMOR:
        return this->_owner.items.armor(static_cast<fb::game::armor*>(item));

    case EQUIPMENT_PARTS::SHIELD:
        return this->_owner.items.shield(static_cast<fb::game::shield*>(item));

    case EQUIPMENT_PARTS::HELMET:
        return this->_owner.items.helmet(static_cast<fb::game::helmet*>(item));

    case EQUIPMENT_PARTS::LEFT_HAND:
        return this->_owner.items.ring(static_cast<fb::game::ring*>(item), EQUIPMENT_POSITION::LEFT);

    case EQUIPMENT_PARTS::RIGHT_HAND:
        return this->_owner.items.ring(static_cast<fb::game::ring*>(item), EQUIPMENT_POSITION::RIGHT);

    case EQUIPMENT_PARTS::LEFT_AUX:
        return this->_owner.items.auxiliary(static_cast<fb::game::auxiliary*>(item), EQUIPMENT_POSITION::LEFT);

    case EQUIPMENT_PARTS::RIGHT_AUX:
        return this->_owner.items.auxiliary(static_cast<fb::game::auxiliary*>(item), EQUIPMENT_POSITION::RIGHT);

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
    fb::game::weapon*       before = this->_weapon;

    this->_weapon = weapon;
    weapon->owner(&this->_owner);
    
    auto listener = this->_owner.get_listener<fb::game::character>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::armor* fb::game::items::armor() const
{
    return this->_armor;
}

fb::game::armor* fb::game::items::armor(fb::game::armor* armor)
{
    fb::game::armor*        before = this->_armor;

    this->_armor = armor;
    armor->owner(&this->_owner);

    auto listener = this->_owner.get_listener<fb::game::character>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::shield* fb::game::items::shield() const
{
    return this->_shield;
}

fb::game::shield* fb::game::items::shield(fb::game::shield* shield)
{
    fb::game::shield*       before = this->_shield;

    this->_shield = shield;
    shield->owner(&this->_owner);

    auto listener = this->_owner.get_listener<fb::game::character>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::helmet* fb::game::items::helmet() const
{
    return this->_helmet;
}

fb::game::helmet* fb::game::items::helmet(fb::game::helmet* helmet)
{
    fb::game::helmet*       before = this->_helmet;

    this->_helmet = helmet;
    helmet->owner(&this->_owner);

    auto listener = this->_owner.get_listener<fb::game::character>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::ring* fb::game::items::ring(EQUIPMENT_POSITION position) const
{
    return this->_rings[static_cast<int>(position)];
}

fb::game::ring* fb::game::items::ring(fb::game::ring* ring)
{
    fb::game::ring*         before = nullptr;

    if(this->_rings[static_cast<int>(EQUIPMENT_POSITION::LEFT)] == nullptr)
    {
        before = this->ring(ring, EQUIPMENT_POSITION::LEFT);
    }
    else
    {
        before = this->ring(ring, EQUIPMENT_POSITION::RIGHT);
    }
    ring->owner(&this->_owner);

    auto listener = this->_owner.get_listener<fb::game::character>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::ring* fb::game::items::ring(fb::game::ring* ring, EQUIPMENT_POSITION position)
{
    auto before = this->_rings[static_cast<int>(position)];
    this->_rings[static_cast<int>(position)] = ring;

    auto listener = this->_owner.get_listener<fb::game::character>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::auxiliary* fb::game::items::auxiliary(EQUIPMENT_POSITION position) const
{
    return this->_auxiliaries[static_cast<int>(position)];
}

fb::game::auxiliary* fb::game::items::auxiliary(fb::game::auxiliary* auxiliary)
{
    fb::game::auxiliary*         before = nullptr;

    if(this->_auxiliaries[static_cast<int>(EQUIPMENT_POSITION::LEFT)] == nullptr)
    {
        before = this->auxiliary(auxiliary, EQUIPMENT_POSITION::LEFT);
    }
    else
    {
        before = this->auxiliary(auxiliary, EQUIPMENT_POSITION::RIGHT);
    }
    auxiliary->owner(&this->_owner);

    auto listener = this->_owner.get_listener<fb::game::character>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::auxiliary* fb::game::items::auxiliary(fb::game::auxiliary* auxiliary, EQUIPMENT_POSITION position)
{
    auto before = this->_auxiliaries[static_cast<int>(position)];
    this->_auxiliaries[static_cast<int>(position)] = auxiliary;

    auto listener = this->_owner.get_listener<fb::game::character>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::item* fb::game::items::find(const std::string& name) const
{
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->at(i);
        if(item == nullptr)
            continue;

        if(item->name() == name)
            return item;
    }

    return nullptr;
}

fb::game::item* fb::game::items::find(const fb::model::item& model) const
{
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->at(i);
        if(item == nullptr)
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
    if(model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
        return nullptr;

    return this->find(model);
}

async::task< fb::game::item*> fb::game::items::drop(uint8_t index, uint8_t count)
{
    auto                        listener = this->_owner.get_listener<fb::game::character>();

    try
    {
        this->_owner.assert_state({STATE::RIDING, STATE::GHOST});
        auto                    item = this->at(index);
        if (item == nullptr)
            co_return nullptr;

        auto&                   model = item->based<fb::model::item>();
        if (model.trade == false)
            throw std::runtime_error(message::exception::CANNOT_DROP_ITEM);

        auto                    dropped = this->remove(*item, count, ITEM_DELETE_TYPE::DROP);
        if(dropped != nullptr)
        {
            co_await dropped->map(this->_owner.map(), this->_owner.position());
            this->_owner.action(ACTION::PICKUP, DURATION::PICKUP);
        }

        co_return dropped;
    }
    catch(std::exception& e)
    {
        this->_owner.message(e.what());
        co_return nullptr;
    }
}

async::task<void> fb::game::items::pickup(bool boost)
{
    auto                    listener = this->_owner.get_listener<fb::game::character>();

    try
    {
        auto                map = this->_owner.map();
        if(map == nullptr)
            co_return;

        this->_owner.action(ACTION::PICKUP, DURATION::PICKUP);


        // Pick up items in reverse order
        auto belows = std::vector<fb::game::item*>();
        for (auto below : map->belows(this->_owner.position(), OBJECT_TYPE::ITEM))
        {
            belows.push_back(static_cast<fb::game::item*>(below));
        }

        std::sort(belows.begin(), belows.end(), [](auto* obj1, auto* obj2)
        {
            auto item1 = static_cast<fb::game::item*>(obj1);
            auto item2 = static_cast<fb::game::item*>(obj2);
            return item1->dropped_time() > item2->dropped_time();
        });

        if (belows.size() == 0)
        {
        }
        else if (boost)
        {
            this->_owner.items.add(belows, true);
        }
        else
        {
            this->_owner.items.add(belows[0]);
        }

        auto thread = lua::get();
        if(thread == nullptr)
            co_return;

        thread->from("scripts/common/pickup.lua")
            .func("on_pickup")
            .pushobject(this->_owner)
            .resume(1);
    }
    catch(std::exception& e)
    {
        this->_owner.message(e.what());
    }
}

async::task<bool> fb::game::items::throws(uint8_t index)
{
    auto listener = this->_owner.get_listener<fb::game::character>();

    try
    {
        auto                    item = this->_owner.items.at(index);
        if(item == nullptr)
            co_return false;

        auto&                   model = item->based<fb::model::item>();
        if(model.trade == false)
            throw std::runtime_error(message::exception::CANNOT_THROW_ITEM);

        auto                    map = this->_owner.map();
        if(map == nullptr)
            throw std::exception();

        auto                    dropped = this->remove(index, 1, ITEM_DELETE_TYPE::THROW);
        auto                    position = this->_owner.position();
        for(int i = 0; i < 7; i++)
        {
            auto                before = position;
            position.forward(this->_owner.direction());
            if(map->movable(position) == false)
            {
                position = before;
                break;
            }
        }

        if(listener != nullptr)
            listener->on_item_throws(this->_owner, *dropped, position);
        co_await dropped->map(map, position);
        co_return true;
    }
    catch(std::exception& e)
    {
        this->_owner.message(e.what());
        co_return false;
    }
}

fb::game::item* fb::game::items::remove(uint8_t index, uint16_t count, ITEM_DELETE_TYPE attr)
{
    auto                    item = this->at(index);
    if(item == nullptr)
        return nullptr;

    auto                    listener = this->_owner.get_listener<fb::game::character>();
    auto                    splitted = item->split(count);
    if(splitted == item)
    {
        fb::game::inventory<fb::game::item>::remove(index);
        if(listener != nullptr)
            listener->on_item_remove(this->_owner, index, attr);
    }

    if(listener != nullptr)
    {
        auto current = this->at(index);
        
        listener->on_item_update(this->_owner, index);
        if(current == nullptr)
            listener->on_item_lost(this->_owner, std::vector<uint8_t> {index});
        else
            listener->on_item_changed(this->_owner, std::map<uint8_t, fb::game::item*> {{index, current}});
    }

    return splitted;
}

fb::game::item* fb::game::items::remove(fb::game::item& item, uint16_t count, ITEM_DELETE_TYPE attr)
{
    auto index = this->index(item);
    if(index == 0xFF)
        return nullptr;

    return this->remove(index, count, attr);
}

bool fb::game::items::swap(uint8_t src, uint8_t dst)
{
    if(fb::game::inventory<fb::game::item>::swap(src, dst) == false)
        return false;
    
    auto listener = this->_owner.get_listener<fb::game::character>();
    if(listener != nullptr)
    {
        const auto              right = this->at(src);
        if(right != nullptr)
            listener->on_item_update(this->_owner, src);
        else
            listener->on_item_remove(this->_owner, src);
        
        const auto              left = this->at(dst);
        if(left != nullptr)
            listener->on_item_update(this->_owner, dst);
        else
            listener->on_item_remove(this->_owner, dst);

        listener->on_item_swap(this->_owner, src, dst);
    }

    return true;
}

std::map<EQUIPMENT_PARTS, fb::game::item*> fb::game::items::equipments() const
{
    return std::map<EQUIPMENT_PARTS, item*>
    {
        {EQUIPMENT_PARTS::WEAPON,      _weapon},
        {EQUIPMENT_PARTS::ARMOR,       _armor},
        {EQUIPMENT_PARTS::SHIELD,      _shield},
        {EQUIPMENT_PARTS::HELMET,      _helmet},
        {EQUIPMENT_PARTS::LEFT_HAND,   _rings[static_cast<int>(EQUIPMENT_POSITION::LEFT)]},
        {EQUIPMENT_PARTS::RIGHT_HAND,  _rings[static_cast<int>(EQUIPMENT_POSITION::RIGHT)]},
        {EQUIPMENT_PARTS::LEFT_AUX,    _auxiliaries[static_cast<int>(EQUIPMENT_POSITION::LEFT)]},
        {EQUIPMENT_PARTS::RIGHT_AUX,   _auxiliaries[static_cast<int>(EQUIPMENT_POSITION::RIGHT)]}
    };
}