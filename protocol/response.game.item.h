#ifndef __PROTOCOL_RESPONSE_GAME_ITEM_H__
#define __PROTOCOL_RESPONSE_GAME_ITEM_H__

#include <protocol/protocol.h>
#include <model/item/item.h>

namespace fb { namespace protocol { namespace response { namespace game { namespace item {

class tip : public fb::protocol::base::response
{
public:
    const uint16_t          position;
    const std::string       message;

public:
    tip(uint16_t position, const std::string& message) : 
        position(position), message(message)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x59)
                  .write_u16(this->position)
                  .write(this->message, true)
                  .write_u8(0x00);
    }
};

class update : public fb::protocol::base::response
{
public:
    const fb::game::session&    me;
    const uint8_t               index;

public:
    update(const fb::game::session& me, uint8_t index) : 
        me(me), index(index)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto                    item = this->me.items.at(index);
        if(item == nullptr)
            return;

        out_stream.write_u8(0x0F)
                  .write_u8(this->index + 1)
                  .write_u16(item->look())
                  .write_u8(item->color())
                  .write(item->name_styled(), false)
                  .write_u32(item->count())
                  .write_u8(0x00)
                  .write_u8(0x00);
    }
};

class update_slot : public fb::protocol::base::response
{
public:
    const fb::game::session&            me;
    const fb::game::equipment::slot     slot;

public:
    update_slot(const fb::game::session& me, fb::game::equipment::slot slot) : 
        me(me), slot(slot)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        fb::game::item*         item;

        switch(slot)
        {
        case equipment::slot::WEAPON_SLOT:
            item = this->me.items.weapon();
            break;

        case equipment::slot::ARMOR_SLOT:
            item = this->me.items.armor();
            break;

        case equipment::slot::SHIELD_SLOT:
            item = this->me.items.shield();
            break;

        case equipment::slot::HELMET_SLOT:
            item = this->me.items.helmet();
            break;

        case equipment::slot::LEFT_HAND_SLOT:
            item = this->me.items.ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT);
            break;

        case equipment::slot::RIGHT_HAND_SLOT:
            item = this->me.items.ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT);
            break;

        case equipment::slot::LEFT_AUX_SLOT:
            item = this->me.items.auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT);
            break;

        case equipment::slot::RIGHT_AUX_SLOT:
            item = this->me.items.auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT);
            break;

        default:
            return;
        }

        if(item == nullptr)
            return;

        out_stream.write_u8(0x37)
                  .write_u16(item->look())
                  .write_u8(item->color())
                  .write(item->name(), false);
    }
};

class remove : public fb::protocol::base::response
{
public:
    const fb::game::item::delete_attr types;
    const uint32_t          index;
    const uint16_t          count;
public:
    remove(fb::game::item::delete_attr types, uint32_t index, uint16_t count = 0) : 
        types(types), index(index), count(count)
    {}
public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x10)
                  .write_u8(this->index + 1)
                  .write_u8(this->types)
                  .write_u16(this->count);
    }
};


class unequip : public fb::protocol::base::response
{
public:
    const equipment::slot   slot;

public:
    unequip(equipment::slot slot) : 
        slot(slot)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x38)
                  .write_u8(this->slot)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_ITEM_H__