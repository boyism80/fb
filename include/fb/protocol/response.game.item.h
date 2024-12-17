#ifndef __PROTOCOL_RESPONSE_GAME_ITEM_H__
#define __PROTOCOL_RESPONSE_GAME_ITEM_H__

#include <fb/protocol/protocol.h>
#include <item.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response::item {

class tip : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x59;

public:
    const uint16_t    position;
    const std::string message;

public:
    tip(uint16_t position, const std::string& message) :
        position(position),
        message(message)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint16_t>(this->position);
        writer.write<std::string, uint16_t>(this->message);
        writer.write<uint8_t>(0x00);
    }
};

class update : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0F;

public:
    const fb::game::character& me;
    const uint8_t              index;

public:
    update(const fb::game::character& me, uint8_t index) :
        me(me),
        index(index)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        auto item = this->me.items.at(index);
        if (item == nullptr)
            co_return;

        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->index + 1);
        writer.write<uint16_t>(item->look());
        writer.write<uint8_t>(item->color());
        writer.write<std::string, uint8_t>(item->inven_name());
        writer.write<uint32_t>(item->count());
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
    }
};

class update_slot : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x37;

public:
    const fb::game::character& me;
    const EQUIPMENT_PARTS      parts;

public:
    update_slot(const fb::game::character& me, EQUIPMENT_PARTS parts) :
        me(me),
        parts(parts)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        fb::game::item* item;

        switch (parts)
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
            co_return;
        }

        if (item == nullptr)
            co_return;

        writer.write<uint8_t>(header);
        writer.write<uint16_t>(item->look());
        writer.write<uint8_t>(item->color());
        writer.write<std::string, uint8_t>(item->name());
    }
};

class remove : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x10;

public:
    const ITEM_DELETE_TYPE type;
    const uint32_t         index;
    const uint16_t         count;

public:
    remove(ITEM_DELETE_TYPE type, uint32_t index, uint16_t count = 0) :
        type(type),
        index(index),
        count(count)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->index + 1);
        writer.write<uint8_t>(static_cast<uint8_t>(this->type));
        writer.write<uint16_t>(this->count);
    }
};

class unequip : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x38;

public:
    const EQUIPMENT_PARTS parts;

public:
    unequip(EQUIPMENT_PARTS parts) :
        parts(parts)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(static_cast<uint8_t>(this->parts));
        writer.write<uint8_t>(0x00);
    }
};

} // namespace fb::protocol::game::response::item

#endif // !__PROTOCOL_RESPONSE_GAME_ITEM_H__