#ifndef __ITEMS_H__
#define __ITEMS_H__

#include "model/item/item.equipment.weapon.h"
#include "model/item/item.equipment.armor.h"
#include "model/item/item.equipment.helmet.h"
#include "model/item/item.equipment.shield.h"
#include "model/item/item.equipment.ring.h"
#include "model/item/item.equipment.auxiliary.h"

namespace fb { namespace game {

class items : public fb::game::base_container<fb::game::item>
{
#pragma region private field
private:
    fb::game::session&                  _owner;

private:
    fb::game::weapon*                   _weapon;
    fb::game::armor*                    _armor;
    fb::game::helmet*                   _helmet;
    fb::game::shield*                   _shield;
    fb::game::ring*                     _rings[2];
    fb::game::auxiliary*                _auxiliaries[2];
#pragma endregion

#pragma region constructor / destructor
public:
    items(session& owner);
    ~items();
#pragma endregion

#pragma region private method
private:
    uint8_t                             equipment_off(fb::game::equipment::slot slot);
#pragma endregion

#pragma region public method
public:
    uint8_t                             add(fb::game::item& item);
    uint8_t                             add(fb::game::item* item);
    std::vector<uint8_t>                add(const std::vector<fb::game::item*>& items);
    uint8_t                             add(fb::game::item& item, uint8_t index);
    bool                                reduce(uint8_t index, uint16_t count);
    fb::game::item*                     active(uint8_t index);
    uint8_t                             inactive(equipment::slot slot);
    uint8_t                             index(const fb::game::item::master* item) const;
    uint8_t                             index(const fb::game::item& item) const;

    fb::game::equipment*                wear(fb::game::equipment::slot slot, fb::game::equipment* item);

    fb::game::weapon*                   weapon() const;
    fb::game::weapon*                   weapon(fb::game::weapon* weapon);

    fb::game::armor*                    armor() const;
    fb::game::armor*                    armor(fb::game::armor* armor);

    fb::game::shield*                   shield() const;
    fb::game::shield*                   shield(fb::game::shield* shield);

    fb::game::helmet*                   helmet() const;
    fb::game::helmet*                   helmet(fb::game::helmet* helmet);

    fb::game::ring*                     ring(equipment::position position) const;
    fb::game::ring*                     ring(fb::game::ring* ring);
    fb::game::ring*                     ring(fb::game::ring* ring, equipment::position position);

    fb::game::auxiliary*                auxiliary(equipment::position position) const;
    fb::game::auxiliary*                auxiliary(fb::game::auxiliary* auxiliary);
    fb::game::auxiliary*                auxiliary(fb::game::auxiliary* auxiliary, equipment::position position);

    fb::game::item*                     find(const std::string& name) const;
    fb::game::item*                     find(const fb::game::item::master& base) const;
    fb::game::item*                     drop(uint8_t index, uint8_t count);
    void                                pickup(bool boost);
    bool                                throws(uint8_t index);

    std::map<equipment::slot, item*>    equipments() const;
#pragma endregion
    
#pragma region override method
    fb::game::item*                     remove(uint8_t index, uint16_t copunt = 1, item::delete_attr attr = item::delete_attr::DELETE_NONE);
    fb::game::item*                     remove(fb::game::item& item, uint16_t count = 1, item::delete_attr attr = item::delete_attr::DELETE_NONE);
    bool                                swap(uint8_t src, uint8_t dst);
#pragma endregion
};

} }

#endif // !__ITEMS_H__