#ifndef __PROTOCOL_RESPONSE_GAME_ITEM_H__
#define __PROTOCOL_RESPONSE_GAME_ITEM_H__

#include <fb/protocol/protocol.h>
#include <fb/game/item.h>
#include <fb/game/model.h>

using namespace fb::model::enum_value;

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
                  .write(item->detailed_name(), false)
                  .write_u32(item->count())
                  .write_u8(0x00)
                  .write_u8(0x00);
    }
};

class update_slot : public fb::protocol::base::header
{
public:
    const fb::game::session&            me;
    const EQUIPMENT_PARTS               parts;

public:
    update_slot(const fb::game::session& me, EQUIPMENT_PARTS parts) : fb::protocol::base::header(0x37),
        me(me), parts(parts)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        fb::game::item*         item;

        switch(parts)
        {
        case EQUIPMENT_PARTS::WEAPON:
            item = this->me.items.weapon();
            break;

        case EQUIPMENT_PARTS::ARMOR:
            item = this->me.items.armor();
            break;

        case EQUIPMENT_PARTS::SHIELD:
            item = this->me.items.shield();
            break;

        case EQUIPMENT_PARTS::HELMET:
            item = this->me.items.helmet();
            break;

        case EQUIPMENT_PARTS::LEFT_HAND:
            item = this->me.items.ring(EQUIPMENT_POSITION::LEFT);
            break;

        case EQUIPMENT_PARTS::RIGHT_HAND:
            item = this->me.items.ring(EQUIPMENT_POSITION::RIGHT);
            break;

        case EQUIPMENT_PARTS::LEFT_AUX:
            item = this->me.items.auxiliary(EQUIPMENT_POSITION::LEFT);
            break;

        case EQUIPMENT_PARTS::RIGHT_AUX:
            item = this->me.items.auxiliary(EQUIPMENT_POSITION::RIGHT);
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
    const ITEM_DELETE_TYPE              type;
    const uint32_t                      index;
    const uint16_t                      count;
public:
    remove(ITEM_DELETE_TYPE type, uint32_t index, uint16_t count = 0) : fb::protocol::base::header(0x10),
        type(type), index(index), count(count)
    { }
public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->index + 1)
                  .write_u8(this->type)
                  .write_u16(this->count);
    }
};


class unequip : public fb::protocol::base::header
{
public:
    const EQUIPMENT_PARTS               parts;

public:
    unequip(EQUIPMENT_PARTS parts) : fb::protocol::base::header(0x38),
        parts(parts)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->parts)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_ITEM_H__