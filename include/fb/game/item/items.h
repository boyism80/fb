#ifndef __ITEM_ITEMS_H__
#define __ITEM_ITEMS_H__

#include <fb/game/inventory.h>
#include <fb/game/item/base.h>
#include <fb/game/item/weapon.h>
#include <fb/game/item/armor.h>
#include <fb/game/item/helmet.h>
#include <fb/game/item/shield.h>
#include <fb/game/item/ring.h>
#include <fb/game/item/auxiliary.h>
#include <memory>
#include <vector>
#include <map>

namespace fb::game {

class items : public fb::game::inventory<fb::game::item>
{
private:
    using super = fb::game::inventory<fb::game::item>;

    // Type aliases for internal use
    using character_ptr   = std::shared_ptr<fb::game::character>;
    using weapon_ptr      = std::shared_ptr<fb::game::weapon>;
    using armor_ptr       = std::shared_ptr<fb::game::armor>;
    using helmet_ptr      = std::shared_ptr<fb::game::helmet>;
    using shield_ptr      = std::shared_ptr<fb::game::shield>;
    using ring_ptr        = std::shared_ptr<fb::game::ring>;
    using auxiliary_ptr   = std::shared_ptr<fb::game::auxiliary>;
    using item_ptr        = std::shared_ptr<fb::game::item>;
    using equipment_ptr   = std::shared_ptr<fb::game::equipment>;
    using equipment_map_t = std::map<EQUIPMENT_PARTS, equipment_ptr>;

private:
    std::weak_ptr<fb::game::character> _owner;
    weapon_ptr                         _weapon         = nullptr;
    armor_ptr                          _armor          = nullptr;
    helmet_ptr                         _helmet         = nullptr;
    shield_ptr                         _shield         = nullptr;
    ring_ptr                           _rings[2]       = {nullptr, nullptr};
    auxiliary_ptr                      _auxiliaries[2] = {nullptr, nullptr};
    std::vector<item_ptr>              _stored;
    uint32_t                           _deposited = 0;

public:
    items();
    ~items();

public:
    // clang-format off
    void                          owner(character_ptr owner);
    character_ptr                 owner() const;
    uint8_t                       add(item_ptr item) override;
    std::vector<uint8_t>          add(const std::vector<item_ptr>& items, bool stop_if_remained = false);
    uint8_t                       add(item_ptr item, uint8_t index) override;
    bool                          store(item_ptr item);
    bool                          store(uint8_t index, uint16_t count);
    bool                          store(std::string_view name, uint16_t count);
    item_ptr                      stored(const fb::model::item& item) const;
    const std::vector<item_ptr>&  stored() const;
    item_ptr                      retrieve(uint8_t index, uint16_t count);
    item_ptr                      retrieve(std::string_view name, uint16_t count);
    item_ptr                      retrieve(const fb::model::item& item, uint16_t count);
    uint32_t                      deposited() const;
    void                          deposited(uint32_t value);
    uint32_t                      deposit(uint32_t value);
    uint32_t                      withdraw(uint32_t value);
    item_ptr                      active(uint8_t index);
    uint8_t                       inactive(EQUIPMENT_PARTS parts);
    equipment_ptr                 equipment_off(EQUIPMENT_PARTS parts);
    uint8_t                       index(const fb::model::item& item) const;
    uint8_t                       index(const item_ptr& item) const;
    std::vector<uint8_t>          index_all(const item_ptr& item) const;
    bool                          update(uint8_t index) const;
    equipment_ptr                 wear(EQUIPMENT_PARTS parts, equipment_ptr item);
    weapon_ptr                    weapon() const;
    weapon_ptr                    weapon(weapon_ptr weapon);
    armor_ptr                     armor() const;
    armor_ptr                     armor(armor_ptr armor);
    shield_ptr                    shield() const;
    shield_ptr                    shield(shield_ptr shield);
    helmet_ptr                    helmet() const;
    helmet_ptr                    helmet(helmet_ptr helmet);
    ring_ptr                      ring(EQUIPMENT_POSITION position) const;
    ring_ptr                      ring(ring_ptr ring);
    ring_ptr                      ring(ring_ptr ring, EQUIPMENT_POSITION position);
    auxiliary_ptr                 auxiliary(EQUIPMENT_POSITION position) const;
    auxiliary_ptr                 auxiliary(auxiliary_ptr auxiliary);
    auxiliary_ptr                 auxiliary(auxiliary_ptr auxiliary, EQUIPMENT_POSITION position);
    item_ptr                      find(std::string_view name) const;
    item_ptr                      find(const fb::model::item& model) const;
    item_ptr                      drop(uint8_t index, uint8_t count, bool action = true, ITEM_DELETE_TYPE delete_type = ITEM_DELETE_TYPE::DROP);
    void                          loot(bool boost);
    bool                          throws(uint8_t index, bool all);
    equipment_map_t               equipments() const;
    bool                          swap(uint8_t src, uint8_t dst) override;
    item_ptr                      remove(uint8_t index, uint16_t count = 1, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE, bool detach = true);
    item_ptr                      remove(item_ptr item, uint16_t count = 1, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE, bool detach = true);
    bool                          is_rewardable(const std::unordered_map<uint32_t, uint16_t>& items, uint32_t money = 0) const;
    bool                          is_rewardable(const std::vector<fb::model::dsl>& items) const;
    // clang-format on
};

} // namespace fb::game

#endif // !__ITEM_ITEMS_H__
