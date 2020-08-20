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
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x59)
                  .write_u16(this->position)
                  .write(this->message, true)
                  .write_u8(0x00);
    }
};


class update_slot : public fb::protocol::base::response
{
public:
    const uint8_t           index;
    const uint16_t          look;
    const uint8_t           color;
    const std::string       name_styled;
    const uint32_t          count;

public:
    update_slot(uint8_t index, uint16_t look, uint8_t color, const std::string& name_styled, uint32_t count) : 
        index(index), 
        look(look), 
        color(color), 
        name_styled(name_styled), 
        count(count)
    {

    }

    // update_slot(items, index)

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x0F)
                  .write_u8(this->index + 1)
                  .write_u16(this->look)
                  .write_u8(this->color)
                  .write(this->name_styled, false)
                  .write_u32(this->count)
                  .write_u8(0x00)
                  .write_u8(0x00);
    }
};


class update_equipment : public fb::protocol::base::response
{
public:
    const uint16_t          look;
    const uint8_t           color;
    const std::string       name;

public:
    update_equipment(uint16_t look, uint8_t color, const std::string& name) : 
        look(look), color(color), name(name)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x37)
                  .write_u16(this->look)
                  .write_u8(this->color)
                  .write(this->name, false);
    }
};


class remove : public fb::protocol::base::response
{
public:
    const fb::game::item::delete_attr types;
    const uint32_t          index;
    const uint16_t          count;
public:
    remove(fb::game::item::delete_attr types, uint32_t index, uint16_t count) : 
        types(types), index(index), count(count)
    {}
public:
    void serialize(fb::ostream& out_stream)
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
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x38)
                  .write_u8(this->slot)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_ITEM_H__