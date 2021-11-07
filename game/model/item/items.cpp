#include "model/item/items.h"
#include "model/session/session.h"
#include "model/map/map.h"

fb::game::items::items(session& owner) :
    base_container(owner, item::MAX_SLOT),
    _owner(static_cast<fb::game::session&>(owner))
{ }

fb::game::items::~items()
{
    if(this->_weapon)
        delete this->_weapon;

    if(this->_armor)
        delete this->_armor;

    if(this->_helmet)
        delete this->_helmet;

    if(this->_shield)
        delete this->_shield;

    if(this->_rings[0])
        delete this->_rings[0];

    if(this->_rings[1])
        delete this->_rings[1];

    if(this->_auxiliaries[0])
        delete this->_auxiliaries[0];

    if(this->_auxiliaries[1])
        delete this->_auxiliaries[1];
}

uint8_t fb::game::items::equipment_off(fb::game::equipment::slot slot)
{
    auto listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        if (this->free() == false)
            throw std::runtime_error(message::exception::INVENTORY_OVERFLOW);

        fb::game::item* item = nullptr;
        switch (slot)
        {
        case fb::game::equipment::slot::WEAPON_SLOT:
            item = this->_weapon;
            if (this->_weapon != nullptr)
                this->_weapon = nullptr;

            break;

        case fb::game::equipment::slot::ARMOR_SLOT:
            item = this->_armor;
            if (this->_armor != nullptr)
                this->_armor = nullptr;
            break;

        case fb::game::equipment::slot::SHIELD_SLOT:
            item = this->_shield;
            if (this->_shield != nullptr)
                this->_shield = nullptr;
            break;

        case fb::game::equipment::slot::HELMET_SLOT:
            item = this->_helmet;
            if (this->_helmet != nullptr)
                this->_helmet = nullptr;
            break;

        case fb::game::equipment::slot::LEFT_HAND_SLOT:
            item = this->_rings[0];
            if (this->_rings[0] != nullptr)
                this->_rings[0] = nullptr;
            break;

        case fb::game::equipment::slot::RIGHT_HAND_SLOT:
            item = this->_rings[1];
            if (this->_rings[1] != nullptr)
                this->_rings[1] = nullptr;
            break;

        case fb::game::equipment::slot::LEFT_AUX_SLOT:
            item = this->_auxiliaries[0];
            if (this->_auxiliaries[0] != nullptr)
                this->_auxiliaries[0] = nullptr;
            break;

        case fb::game::equipment::slot::RIGHT_AUX_SLOT:
            item = this->_auxiliaries[1];
            if (this->_auxiliaries[1] != nullptr)
                this->_auxiliaries[1] = nullptr;
            break;

        default:
            throw std::runtime_error("뭐지 병신 ㅋ");
        }

        auto index = this->add(item);
        if(listener != nullptr)
        {
            listener->on_updated(this->_owner, state_level::LEVEL_MAX);
            listener->on_show(this->_owner, false);
            listener->on_equipment_off(this->_owner, slot, index);
        }
        
        return index;
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(this->_owner, e.what());

        return 0xFF;
    }
}

uint8_t fb::game::items::add(fb::game::item& item)
{
    return this->add(&item);
}

uint8_t fb::game::items::add(fb::game::item* item)
{
    auto result = this->add(std::vector<fb::game::item*> { item });
    if(result.empty())
        return 0xFF;
    else
        return result[0];
}

std::vector<uint8_t> fb::game::items::add(const std::vector<fb::game::item*>& items)
{
    std::vector<uint8_t>                indices;
    std::map<uint8_t, fb::game::item*>  updates;
    auto                                listener = this->_owner.get_listener<fb::game::session>();

    for(auto item : items)
    {
        if(item == nullptr)
            continue;

        auto exists = (fb::game::item*)nullptr;
        if(item->attr(item::attrs::BUNDLE))
        {
            auto exists = this->find(*item->based<fb::game::item>());
            if(exists == nullptr)
            {
                exists = item->split(0);
                this->add(*exists, this->next());
            }

            exists->merge(*item);
            if(item->empty())
                delete item;

            auto index = this->index(*exists);
            
            if(listener != nullptr)
                updates.insert(std::make_pair(index, exists));
            indices.push_back(index);
        }
        else
        {
            auto index = this->next();
            if(index == 0xFF)
                break;

            this->add(*item, index);

            if(item->_map != nullptr)
                item->map(nullptr);

            if(listener != nullptr)
                updates.insert(std::make_pair(index, item));

            indices.push_back(index);
        }
    }

    if(listener != nullptr)
        listener->on_item_changed(this->_owner, updates);

    return std::move(indices);
}

uint8_t fb::game::items::add(fb::game::item& item, uint8_t index)
{
    if(fb::game::base_container<fb::game::item>::add(item, index) == 0xFF)
        return 0xFF;

    item.owner(&this->_owner);

    auto listener = this->_owner.get_listener<fb::game::session>();
    if(listener != nullptr && item.empty() == false)
        listener->on_item_update(static_cast<session&>(this->owner()), index);

    return index;
}

bool fb::game::items::reduce(uint8_t index, uint16_t count)
{
    auto                    item = this->at(index);
    if(item == nullptr)
        return false;

    this->at(index)->reduce(count);
    return true;
}

fb::game::item* fb::game::items::active(uint8_t index)
{
    auto listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        this->_owner.assert_state({state::RIDING, state::GHOST});

        auto                    item = this->at(index);
        if(item == nullptr)
            return nullptr;

        item->active();
        if(item->empty())
        {
            delete item;
            return nullptr;
        }
        else
        {
            return item;
        }
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(this->_owner, e.what());

        return nullptr;
    }
}

uint8_t fb::game::items::inactive(equipment::slot slot)
{
    return this->equipment_off(slot);
}

uint8_t fb::game::items::index(const fb::game::item::master* item) const
{
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        auto now = this->at(i);

        if(now == nullptr)
            continue;

        if(now->based<fb::game::item>() == item)
            return i;
    }

    return 0xFF;
}

uint8_t fb::game::items::index(const fb::game::item& item) const
{
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        auto now = this->at(i);
        if(now == &item)
            return i;
    }

    return 0xFF;
}

fb::game::equipment* fb::game::items::wear(fb::game::equipment::slot slot, fb::game::equipment* item)
{
    switch(slot) // equipment::slot
    {
    case equipment::slot::WEAPON_SLOT:
        return this->_owner.items.weapon(static_cast<fb::game::weapon*>(item));

    case equipment::slot::ARMOR_SLOT:
        return this->_owner.items.armor(static_cast<fb::game::armor*>(item));

    case equipment::slot::SHIELD_SLOT:
        return this->_owner.items.shield(static_cast<fb::game::shield*>(item));

    case equipment::slot::HELMET_SLOT:
        return this->_owner.items.helmet(static_cast<fb::game::helmet*>(item));

    case equipment::slot::LEFT_HAND_SLOT:
        return this->_owner.items.ring(static_cast<fb::game::ring*>(item), equipment::position::LEFT);
        break;

    case equipment::slot::RIGHT_HAND_SLOT:
        return this->_owner.items.ring(static_cast<fb::game::ring*>(item), equipment::position::RIGHT);

    case equipment::slot::LEFT_AUX_SLOT:
        return this->_owner.items.auxiliary(static_cast<fb::game::auxiliary*>(item), equipment::position::LEFT);

    case equipment::slot::RIGHT_AUX_SLOT:
        return this->_owner.items.auxiliary(static_cast<fb::game::auxiliary*>(item), equipment::position::RIGHT);

    default:
        throw std::runtime_error("invalid equipment slot");
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
    
    auto listener = this->_owner.get_listener<fb::game::session>();
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

    auto listener = this->_owner.get_listener<fb::game::session>();
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

    auto listener = this->_owner.get_listener<fb::game::session>();
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

    auto listener = this->_owner.get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::ring* fb::game::items::ring(equipment::position position) const
{
    return this->_rings[static_cast<int>(position)];
}

fb::game::ring* fb::game::items::ring(fb::game::ring* ring)
{
    fb::game::ring*         before = nullptr;

    if(this->_rings[static_cast<int>(equipment::position::LEFT)] == nullptr)
    {
        before = this->ring(ring, equipment::position::LEFT);
    }
    else
    {
        before = this->ring(ring, equipment::position::RIGHT);
    }
    ring->owner(&this->_owner);

    auto listener = this->_owner.get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::ring* fb::game::items::ring(fb::game::ring* ring, equipment::position position)
{
    auto before = this->_rings[static_cast<int>(position)];
    this->_rings[static_cast<int>(position)] = ring;

    auto listener = this->_owner.get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::auxiliary* fb::game::items::auxiliary(equipment::position position) const
{
    return this->_auxiliaries[static_cast<int>(position)];
}

fb::game::auxiliary* fb::game::items::auxiliary(fb::game::auxiliary* auxiliary)
{
    fb::game::auxiliary*         before = nullptr;

    if(this->_auxiliaries[static_cast<int>(equipment::position::LEFT)] == nullptr)
    {
        before = this->auxiliary(auxiliary, equipment::position::LEFT);
    }
    else
    {
        before = this->auxiliary(auxiliary, equipment::position::RIGHT);
    }
    auxiliary->owner(&this->_owner);

    auto listener = this->_owner.get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::auxiliary* fb::game::items::auxiliary(fb::game::auxiliary* auxiliary, equipment::position position)
{
    auto before = this->_auxiliaries[static_cast<int>(position)];
    this->_auxiliaries[static_cast<int>(position)] = auxiliary;

    auto listener = this->_owner.get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_show(this->_owner, true);
    
    return before;
}

fb::game::item* fb::game::items::find(const std::string& name) const
{
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        auto item = this->at(i);
        if(item == nullptr)
            continue;

        if(item->name() == name)
            return item;
    }

    return nullptr;
}

fb::game::item* fb::game::items::find(const fb::game::item::master& base) const
{
    for(int i = 0; i < fb::game::item::MAX_SLOT; i++)
    {
        auto item = this->at(i);
        if(item == nullptr)
            continue;

        if(item->based<fb::game::item>() != &base)
            continue;

        return item;
    }

    return nullptr;
}

fb::game::item* fb::game::items::drop(uint8_t index, uint8_t count)
{
    auto&                       owner = static_cast<fb::game::session&>(this->owner());
    auto                        listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        owner.assert_state({state::RIDING, state::GHOST});

        auto                    dropped = this->remove(index, count, item::delete_attr::DROP);
        if(dropped != nullptr)
        {
            dropped->map(owner.map(), owner.position());
            owner.action(action::PICKUP, duration::DURATION_PICKUP);
        }

        return dropped;
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(owner, e.what());
        return nullptr;
    }
}

void fb::game::items::pickup(bool boost)
{
    auto&                   owner = static_cast<fb::game::session&>(this->owner());
    auto                    listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        auto                map = owner.map();
        if(map == nullptr)
            return;

        owner.action(action::PICKUP, duration::DURATION_PICKUP);


        std::string         message;

        // Pick up items in reverse order
        auto belows = map->belows(this->_owner.position(), fb::game::object::types::ITEM);
        for(int i = belows.size() - 1; i >= 0; i--)
        {
            auto            object = belows[i];
            auto            below = static_cast<fb::game::item*>(object);
            if(below->attr(fb::game::item::attrs::CASH))
            {
                auto        cash = static_cast<fb::game::cash*>(below);
                auto        remain = owner.money_add(cash->chunk());
                cash->chunk(remain); // 먹고 남은 돈으로 설정

                if(listener != nullptr)
                {
                    if(remain != 0)
                        listener->on_notify(this->_owner, message::money::FULL);
                    listener->on_updated(owner, state_level::LEVEL_MIN);
                }
            }
            else
            {
                auto            index = owner.items.add(below);
                if(index == -1)
                    break;
            }

            if(boost == false)
                break;
        }

        lua::get()
            .from("scripts/common/pickup.lua")
            .func("on_pickup")
            .pushobject(owner)
            .resume(1);
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(owner, e.what());
    }
}

bool fb::game::items::throws(uint8_t index)
{
    auto listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        auto                    item = this->_owner.items.at(index);
        if(item == nullptr)
            return false;

        auto                    master = item->based<fb::game::item>();
        if(master->trade.enabled == false)
            throw std::runtime_error(message::exception::CANNOT_THROW_ITEM);

        auto                    map = this->_owner.map();
        if(map == nullptr)
            throw std::exception();

        auto                    dropped = this->remove(index, 1, item::delete_attr::THROW);
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
        
        dropped->map(map, position);
        return true;
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(this->_owner, e.what());
        return false;
    }
}

fb::game::item* fb::game::items::remove(uint8_t index, uint16_t count, item::delete_attr attr)
{
    auto&                   owner = static_cast<fb::game::session&>(this->owner());
    auto                    item = this->at(index);
    if(item == nullptr)
        return nullptr;

    auto                    listener = this->_owner.get_listener<fb::game::session>();
    auto                    splitted = item->split(count);
    if(splitted == item)
    {
        fb::game::base_container<fb::game::item>::remove(index);
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

fb::game::item* fb::game::items::remove(fb::game::item& item, uint16_t count, item::delete_attr attr)
{
    auto index = this->index(item);
    if(index == 0xFF)
        return nullptr;

    return this->remove(index, count, attr);
}

bool fb::game::items::swap(uint8_t src, uint8_t dst)
{
    if(fb::game::base_container<fb::game::item>::swap(src, dst) == false)
        return false;
    
    auto listener = this->_owner.get_listener<fb::game::session>();
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

std::map<fb::game::equipment::slot, fb::game::item*> fb::game::items::equipments() const
{
    return std::map<equipment::slot, item*>
    {
        {equipment::slot::WEAPON_SLOT,      _weapon},
        {equipment::slot::ARMOR_SLOT,       _armor},
        {equipment::slot::SHIELD_SLOT,      _shield},
        {equipment::slot::HELMET_SLOT,      _helmet},
        {equipment::slot::LEFT_HAND_SLOT,   _rings[static_cast<int>(equipment::position::LEFT)]},
        {equipment::slot::RIGHT_HAND_SLOT,  _rings[static_cast<int>(equipment::position::RIGHT)]},
        {equipment::slot::LEFT_AUX_SLOT,    _auxiliaries[static_cast<int>(equipment::position::LEFT)]},
        {equipment::slot::RIGHT_AUX_SLOT,   _auxiliaries[static_cast<int>(equipment::position::RIGHT)]}
    };
}