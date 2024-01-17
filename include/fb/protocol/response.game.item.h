#ifndef __PROTOCOL_RESPONSE_GAME_ITEM_H__
#define __PROTOCOL_RESPONSE_GAME_ITEM_H__

#include <fb/protocol/protocol.h>
#include <fb/game/item.h>

namespace fb { namespace protocol { namespace game { namespace response { namespace item {

class tip : public fb::protocol::base::header
{
public:
    const uint16_t          position;
    const std::string       message;

public:
    tip(uint16_t position, const std::string& message) : fb::protocol::base::header(0x59),
        position(position), message(message)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u16(this->position)
                  .write(this->message, true)
                  .write_u8(0x00);
    }
};

class update : public fb::protocol::base::header
{
public:
    const fb::game::session&    me;
    const uint8_t               index;

public:
    update(const fb::game::session& me, uint8_t index) : fb::protocol::base::header(0x0F),
        me(me), index(index)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto                    item = this->me.items.at(index);
        if(item == nullptr)
            return;

        base::header::serialize(out_stream);
        out_stream.write_u8(this->index + 1)
                  .write_u16(item->look())
                  .write_u8(item->color())
                  .write(item->name_styled(), false)
                  .write_u32(item->count())
                  .write_u8(0x00)
                  .write_u8(0x00);
    }
};

class update_slot : public fb::protocol::base::header
{
public:
    const fb::game::session&            me;
    const fb::game::equipment::slot     slot;

public:
    update_slot(const fb::game::session& me, fb::game::equipment::slot slot) : fb::protocol::base::header(0x37),
        me(me), slot(slot)
    { }

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
            item = this->me.items.ring(equipment::position::LEFT);
            break;

        case equipment::slot::RIGHT_HAND_SLOT:
            item = this->me.items.ring(equipment::position::RIGHT);
            break;

        case equipment::slot::LEFT_AUX_SLOT:
            item = this->me.items.auxiliary(equipment::position::LEFT);
            break;

        case equipment::slot::RIGHT_AUX_SLOT:
            item = this->me.items.auxiliary(equipment::position::RIGHT);
            break;

        default:
            return;
        }

        if(item == nullptr)
            return;

        base::header::serialize(out_stream);
        out_stream.write_u16(item->look())
                  .write_u8(item->color())
                  .write(item->name(), false);
    }
};

class remove : public fb::protocol::base::header
{
public:
    const fb::game::item::DELETE_TYPE   types;
    const uint32_t                      index;
    const uint16_t                      count;
public:
    remove(fb::game::item::DELETE_TYPE types, uint32_t index, uint16_t count = 0) : fb::protocol::base::header(0x10),
        types(types), index(index), count(count)
    { }
public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->index + 1)
                  .write_u8(this->types)
                  .write_u16(this->count);
    }
};


class unequip : public fb::protocol::base::header
{
public:
    const equipment::slot   slot;

public:
    unequip(equipment::slot slot) : fb::protocol::base::header(0x38),
        slot(slot)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->slot)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_ITEM_H__