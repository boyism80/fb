#ifndef __ITEM_ITEMS_H__
#define __ITEM_ITEMS_H__

#include <fb/game/escrow.h>
#include <fb/game/inventory.h>
#include <fb/game/item/armor.h>
#include <fb/game/item/auxiliary.h>
#include <fb/game/item/base.h>
#include <fb/game/item/helmet.h>
#include <fb/game/item/ring.h>
#include <fb/game/item/shield.h>
#include <fb/game/item/weapon.h>

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fb::game {

// `stored` of a saved item row held by an escrow; -1 is the inventory and 0.. is the storage.
static constexpr int16_t ESCROW_STORED = -2;

enum class exchange_result
{
    ok,
    lack_cost,
    lack_capacity,
};

class items : public fb::game::inventory<fb::game::item>
{
public:
    // Held slots stay taken and the client sees them, and held money, as if nothing was held.
    using escrow_type = fb::game::escrow<fb::game::item, CONTAINER_CAPACITY>;

    struct escrow_part
    {
        uint8_t  index = 0;
        uint16_t count = 0;
    };

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

public:
    enum class money_transfer
    {
        DONE,
        LACK_MONEY,
        DEPOSIT_FULL,
        LACK_DEPOSITED,
        MONEY_FULL,
    };

    static constexpr uint64_t MONEY_LIMIT = 0xFFFFFFFF;

private:
    std::weak_ptr<fb::game::character> _owner;
    weapon_ptr                         _weapon         = nullptr;
    armor_ptr                          _armor          = nullptr;
    helmet_ptr                         _helmet         = nullptr;
    shield_ptr                         _shield         = nullptr;
    ring_ptr                           _rings[2]       = {nullptr, nullptr};
    auxiliary_ptr                      _auxiliaries[2] = {nullptr, nullptr};
    std::vector<item_ptr>              _stored;
    uint64_t                           _deposited = 0;
    escrow_type                        _escrow;

public:
    // clang-format off
   items();
   ~items();
    // clang-format on

public:
    // clang-format off
    void                                            owner(character_ptr owner);
    character_ptr                                   owner() const;
    [[nodiscard]] async::task<uint8_t>              add(item_ptr item);
    [[nodiscard]] async::task<std::vector<uint8_t>> add(const std::vector<item_ptr>& items, bool stop_if_remained = false);
    uint8_t                                         add(item_ptr item, uint8_t index) override;
    bool                                            store(item_ptr item);
    [[nodiscard]] async::task<bool>                 store(uint8_t index, uint16_t count);
    [[nodiscard]] async::task<bool>                 store(std::string_view name, uint16_t count);
    item_ptr                                        stored(const fb::model::item& item) const;
    const std::vector<item_ptr>&                    stored() const;
    [[nodiscard]] async::task<item_ptr>             retrieve(size_t index, uint16_t count, bool to_inventory = true);
    uint64_t                                        deposited() const;
    void                                            deposited(uint64_t value);
    money_transfer                                  deposit(uint64_t value);
    money_transfer                                  withdraw(uint64_t value);
    [[nodiscard]] async::task<item_ptr>             active(uint8_t index);
    [[nodiscard]] async::task<uint8_t>              inactive(EQUIPMENT_PARTS parts);
    equipment_ptr                                   equipment_off(EQUIPMENT_PARTS parts);
    uint8_t                                         index(const fb::model::item& item) const;
    uint8_t                                         index(const item_ptr& item) const;
    std::vector<uint8_t>                            index_all(const item_ptr& item) const;
    bool                                            update(uint8_t index) const;
    void                                            load(item_ptr item, const fb::protocol::internal::Item& row);
    void                                            load(const fb::protocol::internal::Escrow& row);
    std::vector<fb::protocol::internal::Item>       to_protocol() const;
    void                                            sync() const;

private:
    void                                            notify_equipment_swap(EQUIPMENT_PARTS parts, const equipment_ptr& before, const equipment_ptr& after);
    void                                            write_item_log(std::string_view event, const fb::game::item& item, uint16_t count, uint8_t index, std::optional<ITEM_DELETE_TYPE> attr = std::nullopt) const;

public:
    weapon_ptr                                      weapon() const;
    weapon_ptr                                      weapon(weapon_ptr weapon);
    armor_ptr                                       armor() const;
    armor_ptr                                       armor(armor_ptr armor);
    shield_ptr                                      shield() const;
    shield_ptr                                      shield(shield_ptr shield);
    helmet_ptr                                      helmet() const;
    helmet_ptr                                      helmet(helmet_ptr helmet);
    ring_ptr                                        ring(EQUIPMENT_POSITION position) const;
    ring_ptr                                        ring(ring_ptr ring);
    ring_ptr                                        ring(ring_ptr ring, EQUIPMENT_POSITION position);
    auxiliary_ptr                                   auxiliary(EQUIPMENT_POSITION position) const;
    auxiliary_ptr                                   auxiliary(auxiliary_ptr auxiliary);
    auxiliary_ptr                                   auxiliary(auxiliary_ptr auxiliary, EQUIPMENT_POSITION position);
    item_ptr                                        find(std::string_view name) const;
    item_ptr                                        find(const fb::model::item& model) const;
    item_ptr                                        find(uint64_t uid) const;
    bool                                            has(const fb::model::item& model, uint16_t count) const;
    uint16_t                                        free_space(const fb::model::item& model) const;
    bool                                            has(const std::vector<std::pair<const fb::model::item*, uint16_t>>& required) const;
    [[nodiscard]] async::task<item_ptr>             drop(uint8_t index, uint16_t count, bool action = true, ITEM_DELETE_TYPE delete_type = ITEM_DELETE_TYPE::DROP);
    [[nodiscard]] async::task<void>                 loot(bool boost);
    [[nodiscard]] async::task<bool>                 throws(uint8_t index, bool all);
    equipment_map_t                                 equipments() const;
    equipment_ptr                                   equipment(EQUIPMENT_PARTS parts) const;
    bool                                            swap(uint8_t src, uint8_t dst) override;
    item_ptr                                        remove(uint8_t index, uint16_t count = 1, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE, bool detach = true);
    item_ptr                                        remove(item_ptr item, uint16_t count = 1, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE, bool detach = true);
    [[nodiscard]] async::task<void>                 remove_expired();
    bool                                            is_rewardable(const std::unordered_map<uint32_t, uint16_t>& items, uint64_t money = 0) const;
    bool                                            is_rewardable(const std::vector<fb::model::dsl>& items) const;
    [[nodiscard]] async::task<exchange_result>      exchange(const std::unordered_map<uint32_t, uint16_t>& cost_items, uint64_t cost_money, const std::unordered_map<uint32_t, uint16_t>& reward_items, uint64_t reward_money, ITEM_DELETE_TYPE delete_type = ITEM_DELETE_TYPE::GIVE);
    [[nodiscard]] async::task<bool>                 combine(const std::vector<uint8_t>& indices);
    bool                                            reserved(uint8_t index) const override;
    const escrow_type&                              escrow() const;
    uint16_t                                        locked_count(const fb::game::item& item) const;
    bool                                            hold(uint64_t escrow_id, uint64_t money, const std::vector<escrow_part>& parts);
    void                                            release(uint64_t escrow_id);
    void                                            take(uint64_t escrow_id);
    void                                            drop_escrow_items();
    // clang-format on
};

} // namespace fb::game

#endif // !__ITEM_ITEMS_H__
