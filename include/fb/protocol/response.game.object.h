#ifndef __PROTOCOL_RESPONSE_GAME_OBJECT_H__
#define __PROTOCOL_RESPONSE_GAME_OBJECT_H__

#include <fb/protocol/protocol.h>
#include <object.h>

using namespace fb::game;

namespace fb::protocol::game::response::object {

class direction : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x11;

public:
#ifndef BOT
    const uint32_t  sequence;
    const DIRECTION value;
#else
    uint32_t  sequence;
    DIRECTION value;
#endif

public:
#ifndef BOT
    direction(const fb::game::object& object) :
        direction(object.sequence(), object.direction())
    { }
    direction(uint32_t sequence, DIRECTION value) :
        sequence(sequence),
        value(value)
    { }
#else
    direction() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->sequence);
        writer.write<uint8_t>(static_cast<uint8_t>(this->value));
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->sequence = reader.read<uint32_t>();
        this->value    = (DIRECTION)reader.read<uint8_t>();
    }
#endif
};

class show : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x07;

private:
    const fb::game::object*               object;
    const std::vector<fb::game::object*>* objects;

public:
    show(const fb::game::object& object) :
        object(&object),
        objects(nullptr)
    { }

    show(const std::vector<fb::game::object*>& objects) :
        object(nullptr),
        objects(&objects)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        if (this->object != nullptr)
        {
            auto map = this->object->map();
            if (map == nullptr)
                co_return;

            writer.write<uint16_t>(0x0001);                                         // count
            writer.write<uint16_t>(this->object->x());                              // object x
            writer.write<uint16_t>(this->object->y());                              // object y
            writer.write<uint32_t>(this->object->sequence());                       // object sequence
            writer.write<uint16_t>(this->object->look());                           // npc icon code
            writer.write<uint8_t>(this->object->color());                           // color
            writer.write<uint8_t>(static_cast<uint8_t>(this->object->direction())); // side
        }
        else if (this->objects != nullptr)
        {
            writer.write<uint8_t>(0x07);
            writer.write<uint16_t>((uint16_t)this->objects->size());

            for (const auto object : *this->objects)
            {
                auto map = object->map();
                if (map == nullptr)
                    continue;

                writer.write<uint16_t>(object->x());                              // object x
                writer.write<uint16_t>(object->y());                              // object y
                writer.write<uint32_t>(object->sequence());                       // object sequence
                writer.write<uint16_t>(object->look());                           // npc icon code
                writer.write<uint8_t>(object->color());                           // color
                writer.write<uint8_t>(static_cast<uint8_t>(object->direction())); // side
            }
        }
        else
        {
            // error
            co_return;
        }
    }
};

class hide : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0E;

public:
    const uint32_t id;

public:
    hide(const fb::game::object& object) :
        hide(object.sequence())
    { }
    hide(uint32_t id) :
        id(id)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->id);
        writer.write<uint8_t>(0x00);
    }
};

class chat : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0D;

public:
    const fb::game::object& me;
    const CHAT_TYPE         type;
    const std::string       message;

public:
    chat(const fb::game::object& me, const CHAT_TYPE type, const std::string message) :
        me(me),
        type(type),
        message(message)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(static_cast<uint8_t>(this->type));
        writer.write<uint32_t>(this->me.sequence());
        writer.write(this->message);
    }
};

class move : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0C;

public:
#ifndef BOT
    const uint32_t  id;
    const point16_t position;
    const DIRECTION direction;
#else
    uint32_t  id;
    point16_t position;
    DIRECTION direction;
#endif

public:
#ifndef BOT
    move(const fb::game::object& object, const point16_t& position) :
        move(object.sequence(), object.direction(), position)
    { }
    move(const uint32_t id, DIRECTION direction, const point16_t& position) :
        id(id),
        direction(direction),
        position(position)
    { }
#else
    move() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->id);
        writer.write<uint16_t>(this->position.x);
        writer.write<uint16_t>(this->position.y);
        writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->id         = reader.read<uint32_t>();
        this->position.x = reader.read<uint16_t>();
        this->position.y = reader.read<uint16_t>();
        this->direction  = (DIRECTION)reader.read<uint8_t>();
    }
#endif
};

class sound : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x19;

public:
    const fb::game::object& me;
    const SOUND             value;

public:
    sound(const fb::game::object& me, SOUND value) :
        me(me),
        value(value)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x03);
        writer.write<uint16_t>(static_cast<uint16_t>(this->value)); // sound
        writer.write<uint8_t>(100);
        writer.write<uint16_t>(0x0004);
        writer.write<uint32_t>(this->me.sequence());
        writer.write<uint16_t>(0x0100);
        writer.write<uint16_t>(0x0202);
        writer.write<uint16_t>(0x0004);
        writer.write<uint16_t>(0xCCCC);
    }
};

class effect : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x29;

public:
    const fb::game::object& me;
    const uint8_t           value;

public:
    effect(const fb::game::object& me, uint8_t value) :
        me(me),
        value(value)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->me.sequence());
        writer.write<uint8_t>(this->value);
        writer.write<uint8_t>(0x00);
    }
};

} // namespace fb::protocol::game::response::object

#endif // !__PROTOCOL_RESPONSE_GAME_OBJECT_H__